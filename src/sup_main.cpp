/////////////////////////////////////////
//  (             (                
//  )\ )     (    )\ )             
// (()/((    )\  (()/(    (   )    
//  /(_))\ )((_)  /(_))  ))\ /((   
// (_))(()/( _   (_))_  /((_|_))\  
// / __|)(_)) |   |   \(_)) _)((_) 
// \__ \ || | |   | |) / -_)\ V /  
// |___/\_, |_|   |___/\___| \_/   
//      |__/                       
//
// 2014 Moritz Kretz
/////////////////////////////////////////
// Cerb3rus
// -- I don't have fancy ASCII art :( --
// 2014 David Lehn
/////////////////////////////////////////

#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <shlobj.h>

#include "Shlwapi.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma pack(1)

namespace SUP
{
	enum ScreenCorner
	{
		TopLeft = 0,
		TopRight,
		BottomRight,
		BottomLeft
	};

	HINSTANCE hInst = NULL;
	HINSTANCE hLib = 0;
	FARPROC p[14] = {0};

	DWORD hookTreadId = NULL;
	HWND hWnd = NULL;
	LONG oldWndProc;
	HMENU hMenu = NULL;
	HWINEVENTHOOK hRichEditShowHook = NULL;

	HMENU hUtilMenu = NULL;
	HMENU hDisplayMenu = NULL;
	HMENU hPosMenu = NULL;

#define ID_ENABLE_CHAT_FORMAT 2000
#define ID_SET_NOTIFICATION_DISPLAY 2100
#define ID_SET_NOTIFICATION_POS 2200
#define ID_SHOW_CREDITS_MOE 2300
#define ID_SHOW_CREDITS_DAVE 2301
	
#define WM_REGISTER_SHOW_HOOK WM_APP - 1

	std::wstring iniPath;
	bool enableChatFormat = true;
	ScreenCorner notifCorner = BottomRight;
	std::wstring notifDisplay = L"";
	int notifOffsetX = 0;
	int notifOffsetY = 0;

	std::vector<std::wstring> displayNames;

	HWND findWindowInProcess(const wchar_t* _wndClass, const wchar_t* _wndTitle)
	{
		DWORD procId = GetCurrentProcessId();
		DWORD wndProcId = NULL;
		HWND result = NULL;
		do
		{
			result = FindWindowEx(NULL, result, _wndClass, _wndTitle);

			if (result == NULL)
				break;

			wndProcId = NULL;
			GetWindowThreadProcessId(result, &wndProcId);
		} while (wndProcId != procId);

		return result;
	}

	// Returns the first visible child window of _parent matching the criteria. If no visible
	// child windows are available, the first found child window will be returned. If no
	// matching child windows were found at all, returns NULL.
	HWND findVisibleChild(HWND _parent, const wchar_t* _wndClass, const wchar_t* _wndTitle)
	{
		HWND resultFirst = FindWindowEx(_parent, NULL, _wndClass, _wndTitle);
		HWND result = resultFirst;
		while (result != NULL && IsWindowVisible(result) == FALSE)
		{
			result = FindWindowEx(_parent, result, _wndClass, _wndTitle);

			if (result == NULL)
			{
				result = resultFirst;
				break;
			}
		}

		return result;
	}

	void createMenus(HMENU _parent)
	{
		hUtilMenu = CreateMenu();
		AppendMenu(_parent, MF_STRING | MF_POPUP, (UINT_PTR)hUtilMenu, L"&Util");

		UINT flags = MF_STRING | (enableChatFormat ? MF_CHECKED : MF_UNCHECKED);

		AppendMenu(hUtilMenu, flags, ID_ENABLE_CHAT_FORMAT, L"&Allow Chat Formatting");

		HMENU notifMenu = CreateMenu();
		AppendMenu(hUtilMenu, MF_STRING | MF_POPUP, (UINT_PTR)notifMenu,
			L"Show &Notifications");

		hDisplayMenu = CreateMenu();
		AppendMenu(notifMenu, MF_STRING | MF_POPUP, (UINT_PTR)hDisplayMenu,
			L"On &Display");

		hPosMenu = CreateMenu();
		AppendMenu(notifMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPosMenu,
			L"At &Location");

		flags = MF_STRING | MF_UNCHECKED;
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + TopLeft, L"&Top Left");
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + TopRight, L"T&op Right");
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + BottomRight, L"Bottom &Right");
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + BottomLeft, L"Bottom &Left");

		HMENU creditsMenu = CreateMenu();
		AppendMenu(hUtilMenu, MF_STRING | MF_POPUP, (UINT_PTR)creditsMenu,
			L"&Credits");
		AppendMenu(creditsMenu, MF_STRING, ID_SHOW_CREDITS_DAVE, L"&David Lehn");
		AppendMenu(creditsMenu, MF_STRING, ID_SHOW_CREDITS_MOE, L"&Moritz Kretz");
	}

	void updatePosMenu()
	{
		for (unsigned i = 0; i < 4; i++)
		{
			CheckMenuItem(hPosMenu, ID_SET_NOTIFICATION_POS + i,
				(i == notifCorner) ? MF_CHECKED : MF_UNCHECKED);
		}
	}

	void updateDisplayMenu()
	{
		int items = GetMenuItemCount(hDisplayMenu);
		for (int i = items - 1; i >= 0; i--)
		{
			DeleteMenu(hDisplayMenu, i, MF_BYPOSITION);
		}

		displayNames.clear();

		static auto enumDisplays = [](HMONITOR _monitor, HDC _hdc, LPRECT _rect, LPARAM _param)
			-> BOOL
		{
			MONITORINFOEX info;
			info.cbSize = sizeof(MONITORINFOEX);

			if (GetMonitorInfo(_monitor, &info))
				displayNames.push_back(info.szDevice);

			return TRUE;
		};

		EnumDisplayMonitors(NULL, NULL, enumDisplays, NULL);

		for (unsigned i = 0; i < displayNames.size(); i++)
		{
			UINT flags = MF_STRING
				| ((displayNames[i] == notifDisplay) ? MF_CHECKED : MF_UNCHECKED);
			AppendMenu(hDisplayMenu, flags, ID_SET_NOTIFICATION_DISPLAY + i,
				displayNames[i].c_str());
		}
	}

	POINT calcWindowPos(const RECT& _base)
	{
		RECT workArea = {0};

		static auto enumDisplays = [](HMONITOR _monitor, HDC _hdc, LPRECT _rect, LPARAM _param)
			-> BOOL
		{
			RECT& r = *(RECT*)_param;
			MONITORINFOEX info;
			info.cbSize = sizeof(MONITORINFOEX);

			if (!GetMonitorInfo(_monitor, &info))
				TRUE;

			r = info.rcWork;

			return info.szDevice != notifDisplay;
		};
		EnumDisplayMonitors(NULL, NULL, enumDisplays, (LPARAM)&workArea);

		int width = _base.right - _base.left, height = _base.bottom - _base.top;

		POINT result;
		if (notifCorner == TopLeft || notifCorner == BottomLeft)
			result.x = workArea.left + notifOffsetX;
		else
			result.x = workArea.right - width - notifOffsetX;

		if (notifCorner == TopLeft || notifCorner == TopRight)
			result.y = workArea.top + notifOffsetY;
		else
			result.y = workArea.bottom - height - notifOffsetY;

		return result;
	}

	void resetRichEditShowHook()
	{
		PostThreadMessage(hookTreadId, WM_REGISTER_SHOW_HOOK, 42, 42);
	}

	void applyChatFormat(HWND _hChatRichEdit)
	{
		wchar_t buffer[8192];
		GetWindowText(_hChatRichEdit, buffer, 8192);

		if (enableChatFormat)
			SetWindowText(_hChatRichEdit, L"/setupkey *Lib/Conversation/EnableWiki 1");
		else
			SetWindowText(_hChatRichEdit, L"/setupkey *Lib/Conversation/EnableWiki 0");

		SendMessage(_hChatRichEdit, WM_KEYDOWN, VK_RETURN, 0);
		SendMessage(_hChatRichEdit, WM_KEYUP, VK_RETURN, 0);

		SetWindowText(_hChatRichEdit, buffer);
	}

	void chatFormatChanged()
	{
		if (enableChatFormat)
			CheckMenuItem(hUtilMenu, 0, MF_CHECKED | MF_BYPOSITION);
		else
			CheckMenuItem(hUtilMenu, 0, MF_UNCHECKED | MF_BYPOSITION);

		WritePrivateProfileString(L"config", L"enableChatFormat",
			std::to_wstring(enableChatFormat ? 1 : 0).c_str(), iniPath.c_str());

		HWND hWndChat = findWindowInProcess(L"TConversationForm", nullptr);

		if (hWndChat == NULL)
		{
			// Combined View
			hWndChat = findVisibleChild(hWnd, L"TConversationForm", nullptr);

			if (hWndChat == NULL)
			{
				resetRichEditShowHook();
				return;
			}
		}

		HWND hWndControl = findVisibleChild(hWndChat, L"TChatEntryControl", nullptr);

		if (hWndControl == NULL)
		{
			resetRichEditShowHook();
			return;
		}

		HWND hWndText = findVisibleChild(hWndControl, L"TChatRichEdit", nullptr);
		if (hWndText == NULL)
		{
			resetRichEditShowHook();
			return;
		}

		applyChatFormat(hWndText);
	}

	LRESULT CALLBACK newWndProc(HWND _hwnd, UINT _message, WPARAM _wParam, LPARAM _lParam)
	{
		switch (_message)
		{
		case WM_ERASEBKGND:
		{
			// This message happens to be sent when the user changes Skype's layout. We can use
			// this to check whether we need to reattach our custom menu.
			HMENU hCurrent = GetMenu(_hwnd);
			if (hCurrent != hMenu)
			{
				createMenus(hCurrent);
				hMenu = hCurrent;
			}
			break;
		}
		case WM_INITMENUPOPUP:
			if ((HMENU)_wParam == hPosMenu)
				updatePosMenu();
			else if ((HMENU)_wParam == hDisplayMenu)
				updateDisplayMenu();
			break;
		case WM_COMMAND:
			if (_wParam == ID_ENABLE_CHAT_FORMAT)
			{
				enableChatFormat = !enableChatFormat;
				chatFormatChanged();
			}
			else if (_wParam == ID_SHOW_CREDITS_DAVE)
			{
				ShellExecute(NULL, L"open", L"http://blog.mountain-view.de/", nullptr, nullptr,
					SW_SHOWNORMAL);
			}
			else if (_wParam == ID_SHOW_CREDITS_MOE)
			{
				ShellExecute(NULL, L"open", L"http://kretzmoritz.wordpress.com/", nullptr,
					nullptr, SW_SHOWNORMAL);
			}
			else if (_wParam >= ID_SET_NOTIFICATION_DISPLAY
				&& _wParam < ID_SET_NOTIFICATION_DISPLAY + displayNames.size())
			{
				notifDisplay = displayNames[_wParam - ID_SET_NOTIFICATION_DISPLAY];
				WritePrivateProfileString(L"config", L"notifDisplay",
					notifDisplay.c_str(), iniPath.c_str());
			}
			else if (_wParam >= ID_SET_NOTIFICATION_POS
				&& _wParam < ID_SET_NOTIFICATION_POS + 4)
			{
				notifCorner = (ScreenCorner)(_wParam - ID_SET_NOTIFICATION_POS);
				WritePrivateProfileString(L"config", L"notifCorner",
					std::to_wstring((int)notifCorner).c_str(), iniPath.c_str());
			}
			break;
		}

		return CallWindowProc((WNDPROC)oldWndProc, _hwnd, _message, _wParam, _lParam);
	}

	void APIENTRY notificationHook(HWINEVENTHOOK _hWinEventHook, DWORD _event, HWND _hwnd,
		LONG _idObject, LONG _idChild, DWORD _idEventThread, DWORD _dwmsEventTime)
	{
		switch (_event)
		{
		case EVENT_OBJECT_SHOW:
		{
			wchar_t buffer[MAX_CLASS_NAME] = {0};
			GetClassName(_hwnd, buffer, MAX_CLASS_NAME);
			std::wstring className = buffer;

			if (className == L"TChatRichEdit")
			{
				// After this, we don't need this hook to be called anymore.
				UnhookWinEvent(_hWinEventHook);
				hRichEditShowHook = NULL;
				// Once a chat edit field is available, we use it to set our value.
				applyChatFormat(_hwnd);
			}
			break;
		}
		case EVENT_OBJECT_LOCATIONCHANGE:
		{
			static const std::wstring trayAlertClass = L"TTrayAlert";
			wchar_t className[MAX_CLASS_NAME] = {0};
			GetClassName(_hwnd, className, MAX_CLASS_NAME);

			if (className != trayAlertClass)
				return;

			RECT rect;
			if (!GetWindowRect(_hwnd, &rect))
				return;

			POINT pos = calcWindowPos(rect);
			MoveWindow(_hwnd, pos.x, pos.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
			break;
		}
		}
	}

	DWORD WINAPI hook()
	{
		wchar_t buffer[MAX_PATH];

		DWORD procId = GetCurrentProcessId();
		resetRichEditShowHook();
		SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE,
			EVENT_OBJECT_LOCATIONCHANGE, hInst,
			notificationHook, procId, NULL,
			WINEVENT_INCONTEXT | WINEVENT_SKIPOWNTHREAD);

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, buffer)))
			iniPath = std::wstring(buffer) + L"\\Skype\\sup.ini";
		else
		{
			GetModuleFileName((HMODULE)hInst, buffer, MAX_PATH);
			PathRemoveFileSpec(buffer);

			iniPath = std::wstring(buffer) + L"\\sup.ini";
		}

		enableChatFormat = GetPrivateProfileInt(L"config", L"enableChatFormat", 1,
			iniPath.c_str()) != 0;
		notifCorner = (ScreenCorner)GetPrivateProfileInt(L"config", L"notifCorner", BottomRight,
			iniPath.c_str());
		GetPrivateProfileString(L"config", L"notifDisplay", L"", buffer, MAX_PATH,
			iniPath.c_str());
		notifDisplay = buffer;
		notifOffsetX = GetPrivateProfileInt(L"config", L"notifOffsetX", 0, iniPath.c_str());
		notifOffsetY = GetPrivateProfileInt(L"config", L"notifOffsetY", 0, iniPath.c_str());

		while (hWnd == NULL)
			hWnd = findWindowInProcess(L"tSkMainForm", nullptr);

		oldWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)newWndProc);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			switch (msg.message)
			{
			case WM_REGISTER_SHOW_HOOK:
				if (hRichEditShowHook == NULL)
				{
					hRichEditShowHook = SetWinEventHook(EVENT_OBJECT_SHOW,
						EVENT_OBJECT_SHOW, hInst,
						notificationHook, procId, NULL,
						WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNTHREAD);
				}
				break;
			}
		}

		return TRUE;
	}
}

BOOL WINAPI DllMain(HINSTANCE _hInst, DWORD _reason, LPVOID _reserved)
{
	if (_reason == DLL_PROCESS_ATTACH)
	{
		SUP::hInst = _hInst;

		wchar_t buffer[MAX_PATH];
		GetSystemDirectory(buffer, MAX_PATH);
		std::wstring path = std::wstring(buffer) + L"\\d3d9.dll";

		SUP::hLib = LoadLibrary(path.c_str());
		
		if (!SUP::hLib)
		{
			return FALSE;
		}

		SUP::p[0] = GetProcAddress(SUP::hLib, "D3DPERF_BeginEvent");
		SUP::p[1] = GetProcAddress(SUP::hLib, "D3DPERF_EndEvent");
		SUP::p[2] = GetProcAddress(SUP::hLib, "D3DPERF_GetStatus");
		SUP::p[3] = GetProcAddress(SUP::hLib, "D3DPERF_QueryRepeatFrame");
		SUP::p[4] = GetProcAddress(SUP::hLib, "D3DPERF_SetMarker");
		SUP::p[5] = GetProcAddress(SUP::hLib, "D3DPERF_SetOptions");
		SUP::p[6] = GetProcAddress(SUP::hLib, "D3DPERF_SetRegion");
		SUP::p[7] = GetProcAddress(SUP::hLib, "DebugSetLevel");
		SUP::p[8] = GetProcAddress(SUP::hLib, "DebugSetMute");
		SUP::p[9] = GetProcAddress(SUP::hLib, "Direct3DCreate9");
		SUP::p[10] = GetProcAddress(SUP::hLib, "Direct3DCreate9Ex");
		SUP::p[11] = GetProcAddress(SUP::hLib, "Direct3DShaderValidatorCreate9");
		SUP::p[12] = GetProcAddress(SUP::hLib, "PSGPError");
		SUP::p[13] = GetProcAddress(SUP::hLib, "PSGPSampleTexture");

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SUP::hook, 0, 0, &SUP::hookTreadId);
	}
	else if (_reason == DLL_PROCESS_DETACH)
	{
		FreeLibrary(SUP::hLib);
	}

	return TRUE;
}

// D3DPERF_BeginEvent
extern "C" __declspec(naked) void __stdcall __E__0__()
{
	__asm
	{
		jmp SUP::p[0 * 4];
	}
}

// D3DPERF_EndEvent
extern "C" __declspec(naked) void __stdcall __E__1__()
{
	__asm
	{
		jmp SUP::p[1 * 4];
	}
}

// D3DPERF_GetStatus
extern "C" __declspec(naked) void __stdcall __E__2__()
{
	__asm
	{
		jmp SUP::p[2 * 4];
	}
}

// D3DPERF_QueryRepeatFrame
extern "C" __declspec(naked) void __stdcall __E__3__()
{
	__asm
	{
		jmp SUP::p[3 * 4];
	}
}

// D3DPERF_SetMarker
extern "C" __declspec(naked) void __stdcall __E__4__()
{
	__asm
	{
		jmp SUP::p[4 * 4];
	}
}

// D3DPERF_SetOptions
extern "C" __declspec(naked) void __stdcall __E__5__()
{
	__asm
	{
		jmp SUP::p[5 * 4];
	}
}

// D3DPERF_SetRegion
extern "C" __declspec(naked) void __stdcall __E__6__()
{
	__asm
	{
		jmp SUP::p[6 * 4];
	}
}

// DebugSetLevel
extern "C" __declspec(naked) void __stdcall __E__7__()
{
	__asm
	{
		jmp SUP::p[7 * 4];
	}
}

// DebugSetMute
extern "C" __declspec(naked) void __stdcall __E__8__()
{
	__asm
	{
		jmp SUP::p[8 * 4];
	}
}

// Direct3DCreate9
extern "C" __declspec(naked) void __stdcall __E__9__()
{
	__asm
	{
		jmp SUP::p[9 * 4];
	}
}

// Direct3DCreate9Ex
extern "C" __declspec(naked) void __stdcall __E__10__()
{
	__asm
	{
		jmp SUP::p[10 * 4];
	}
}

// Direct3DShaderValidatorCreate9
extern "C" __declspec(naked) void __stdcall __E__11__()
{
	__asm
	{
		jmp SUP::p[11 * 4];
	}
}

// PSGPError
extern "C" __declspec(naked) void __stdcall __E__12__()
{
	__asm
	{
		jmp SUP::p[12 * 4];
	}
}

// PSGPSampleTexture
extern "C" __declspec(naked) void __stdcall __E__13__()
{
	__asm
	{
		jmp SUP::p[13 * 4];
	}
}