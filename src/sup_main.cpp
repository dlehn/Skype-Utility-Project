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
#include <Shlwapi.h>

#include "sup_constants.h"
#include "sup_chat_commands.h"
#include "sup_window_util.h"
#include "../res/sup_resource.h"

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

	HWND hWnd = NULL;
	LONG oldWndProc;
	HMENU hMenu = NULL;

	HMENU hUtilMenu = NULL;
	HMENU hLayoutMenu = NULL;
	HMENU hLanguageMenu = NULL;
	HMENU hDisplayMenu = NULL;
	HMENU hPosMenu = NULL;
	HMENU hSkypeMenuAttachTo = NULL;
	
	ChatCommandHandler commandHandler;

	std::wstring iniPath;
	bool enableChatFormat = true;
	bool hideAds = false;
	bool hideAppToolbar = false;
	bool hideIdentityPanel = false;
	ScreenCorner notifCorner = BottomRight;
	std::wstring notifDisplay = L"";
	int languageId = 0;
	int notifOffsetX = 0;
	int notifOffsetY = 0;
	int viewChangeUpdateDelay = 500;

	std::vector<std::wstring> displayNames;
	int appToolbarHeight = 0;
	int identityPanelHeight = 0;
	// Variable used to ensure that forced layout is only applied after the Skype contact list
	// has been displayed for the first time. Otherwise Skype will crash or freeze in Version
	// 7.1.0.105 if the "Hide Identity Panel" option is enabled.
	bool allowHideIdentityPanel = false;

	WORD LoadStringLang(UINT _strID, LPTSTR _destStr, WORD _strLen)
	{
		HRSRC hrRes = FindResourceEx(hInst, RT_STRING, MAKEINTRESOURCE(1+(_strID >> 4)),
			MAKELANGID(languageId, 0));
		LPCWSTR str = (LPCWSTR)LoadResource(hInst, hrRes);
		
		if (!str)
			return 0;

		for (WORD strPos = 0; strPos < (_strID & 0x000F); strPos++)
			str += *str + 1;
		
		if (!_strLen)
			return *str;
		
		if (!_destStr)
			return 0;

		int len = min(_strLen, *str+1);

		lstrcpyn(_destStr, str + 1, len);
		_destStr[len - 1] = TEXT('\0');

		return len;
	}

	void createMenus(HMENU _parent)
	{
		WCHAR buffer[200];

		hUtilMenu = CreateMenu();

		UINT flags = MF_STRING | MF_UNCHECKED;
		LoadStringLang(IDS_MENU_ALLOW_CHAT_FORMATING, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hUtilMenu, flags, ID_ENABLE_CHAT_FORMAT, buffer);

		hLayoutMenu = CreateMenu();
		LoadStringLang(IDS_MENU_LAYOUT, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hUtilMenu, MF_STRING | MF_POPUP, (UINT_PTR)hLayoutMenu, buffer);

		LoadStringLang(IDS_MENU_HIDE_ADS, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hLayoutMenu, flags, ID_HIDE_ADS, buffer);

		LoadStringLang(IDS_MENU_HIDE_APP_TOOLBAR, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hLayoutMenu, flags, ID_HIDE_APP_TOOLBAR, buffer);

		LoadStringLang(IDS_MENU_HIDE_IDENTITY_PANEL, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hLayoutMenu, flags, ID_HIDE_IDENTITY_PANEL, buffer);

		HMENU notifMenu = CreateMenu();
		LoadStringLang(IDS_MENU_NOTIFICATION, (LPTSTR) &buffer, sizeof(buffer));
		AppendMenu(hUtilMenu, MF_STRING | MF_POPUP, (UINT_PTR)notifMenu, buffer);

		hDisplayMenu = CreateMenu();
		LoadStringLang(IDS_MENU_ON_DISPLAY, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(notifMenu, MF_STRING | MF_POPUP, (UINT_PTR)hDisplayMenu, buffer);

		hPosMenu = CreateMenu();
		LoadStringLang(IDS_MENU_AT_LOCATION, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(notifMenu, MF_STRING | MF_POPUP, (UINT_PTR)hPosMenu, buffer);
		
		LoadStringLang(IDS_MENU_NOTIFICATION_POS_TOPLEFT, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + TopLeft, buffer);
		LoadStringLang(IDS_MENU_NOTIFICATION_POS_TOPRIGHT, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + TopRight, buffer);
		LoadStringLang(IDS_MENU_NOTIFICATION_POS_BOTTOMRIGHT, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + BottomRight, buffer);
		LoadStringLang(IDS_MENU_NOTIFICATION_POS_BOTTOMLEFT, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hPosMenu, flags, ID_SET_NOTIFICATION_POS + BottomLeft, buffer);

		hLanguageMenu = CreateMenu();
		LoadStringLang(IDS_MENU_LANGUAGE, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hUtilMenu, MF_STRING | MF_POPUP, (UINT_PTR)hLanguageMenu, buffer);

		LoadStringLang(IDS_MENU_LANGUAGE_ENGLISH, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hLanguageMenu, flags, ID_LANGUAGE_ENGLISH, buffer);

		LoadStringLang(IDS_MENU_LANGUAGE_RUSSIAN, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hLanguageMenu, flags, ID_LANGUAGE_RUSSIAN, buffer);

		LoadStringLang(IDS_MENU_LANGUAGE_GERMAN, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hLanguageMenu, flags, ID_LANGUAGE_GERMAN, buffer);

		HMENU helpMenu = CreateMenu();
		LoadStringLang(IDS_MENU_HELP, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(hUtilMenu, MF_STRING | MF_POPUP, (UINT_PTR)helpMenu,	buffer);

		LoadStringLang(IDS_MENU_SHOW_HELP, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(helpMenu, MF_STRING, ID_SHOW_HELP, buffer);
		LoadStringLang(IDS_MENU_SHOW_UPDATES, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(helpMenu, MF_STRING, ID_SHOW_UPDATES, buffer);
		LoadStringLang(IDS_MENU_VERSION, (LPTSTR)&buffer, sizeof(buffer));
		wcscat_s(buffer, SUP_VERSION);
		AppendMenu(helpMenu, MF_STRING | MF_DISABLED, 0, buffer);

		HMENU creditsMenu = CreateMenu();
		LoadStringLang(IDS_MENU_CREDITS, (LPTSTR)&buffer, sizeof(buffer));
		AppendMenu(helpMenu, MF_STRING | MF_POPUP, (UINT_PTR)creditsMenu, buffer);
		AppendMenu(creditsMenu, MF_STRING, ID_SHOW_CREDITS_DAVE, L"&David Lehn");
		AppendMenu(creditsMenu, MF_STRING, ID_SHOW_CREDITS_MOE, L"&Moritz Kretz");
		AppendMenu(creditsMenu, MF_STRING, ID_SHOW_CREDITS_NDS, L"&NDS");
	}

	void attachMenu()
	{
		WCHAR buffer[200];
		int count = 0;

		if (!hSkypeMenuAttachTo)
			return;

		if ((count = GetMenuItemCount(hSkypeMenuAttachTo)) <= 0)
			return;

		// Check if our custom menu attached already
		if (GetSubMenu(hSkypeMenuAttachTo, count - 1) == hUtilMenu)
			return;

		AppendMenu(hSkypeMenuAttachTo, MF_SEPARATOR, NULL, NULL);

		LoadStringLang(IDS_MAINMENU, (LPTSTR) &buffer, sizeof(buffer));
		AppendMenu(hSkypeMenuAttachTo, MF_STRING | MF_POPUP, (UINT_PTR) hUtilMenu, buffer);
	}

	void updateUtilMenu()
	{
		CheckMenuItem(hUtilMenu, ID_ENABLE_CHAT_FORMAT,
			enableChatFormat ? MF_UNCHECKED : MF_CHECKED);
	}

	void updateLayoutMenu()
	{
		CheckMenuItem(hLayoutMenu, ID_HIDE_ADS,
			hideAds ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hLayoutMenu, ID_HIDE_APP_TOOLBAR,
			hideAppToolbar ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hLayoutMenu, ID_HIDE_IDENTITY_PANEL,
			hideIdentityPanel ? MF_CHECKED : MF_UNCHECKED);
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

	void updateLanguageMenu()
	{
		int items;
		UINT sub;

		if ((items = GetMenuItemCount(hLanguageMenu)) <= 0)
			return;

		for (int i = 0; i < items; i++)
		{
			if ((sub = GetMenuItemID(hLanguageMenu, i)) < 0)
				continue;

			CheckMenuItem(hLanguageMenu, sub,
				(sub == (ID_LANGUAGE_BASEID + languageId)) ? MF_CHECKED : MF_UNCHECKED);
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

	void chatFormatChanged()
	{
		WritePrivateProfileString(L"config", L"enableChatFormat",
			std::to_wstring(enableChatFormat ? 1 : 0).c_str(), iniPath.c_str());

		std::wstring cmd = L"/setupkey *Lib/Conversation/EnableWiki ";
		cmd += enableChatFormat ? L'1' : L'0';

		commandHandler.queueCommand(cmd, L"changeChatFormat", true);
	}

	void hideAdsChanged()
	{
		WritePrivateProfileString(L"config", L"hideAds", hideAds ? L"1" : L"0",
			iniPath.c_str());

		if (!hideAds)
		{
			forceLayoutUpdate(hWnd);
			HWND parent = NULL;
			while (true)
			{
				parent = findWindowInProcess(CLS_CONVERSATION_FORM.c_str(), nullptr, parent);
				if (!parent)
					break;
				forceLayoutUpdate(parent);
			}
			return;
		}

		HWND banner = NULL;
		while (true)
		{
			banner = FindWindowEx(hWnd, banner, CLS_CHAT_BANNER.c_str(), nullptr);
			if (!banner)
				break;

			SetWindowPos(banner, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// Combined view
		HWND parent = NULL;
		while (true)
		{
			parent = FindWindowEx(hWnd, parent, CLS_CONVERSATION_FORM.c_str(), nullptr);
			if (!parent)
				break;

			banner = NULL;
			while (true)
			{
				banner = FindWindowEx(hWnd, banner, CLS_CHAT_BANNER.c_str(), nullptr);
				if (!banner)
					break;

				SetWindowPos(banner, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
		forceLayoutUpdate(hWnd);

		// Split view
		parent = NULL;
		while (true)
		{
			parent = findWindowInProcess(CLS_CONVERSATION_FORM.c_str(), nullptr, parent);
			if (!parent)
				break;

			banner = NULL;
			while (true)
			{
				banner = FindWindowEx(hWnd, banner, CLS_CHAT_BANNER.c_str(), nullptr);
				if (!banner)
					break;

				SetWindowPos(banner, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
			forceLayoutUpdate(parent);
		}
	}

	void hideAppToolbarChanged()
	{
		WritePrivateProfileString(L"config", L"hideAppToolbar", hideAppToolbar ? L"1" : L"0",
			iniPath.c_str());

		HWND appToolbar = FindWindowEx(hWnd, NULL, CLS_APP_TOOLBAR.c_str(), nullptr);
		if (!appToolbar)
			return;

		RECT r;
		GetWindowRect(appToolbar, &r);

		if (hideAppToolbar)
		{
			appToolbarHeight = r.bottom - r.top;
			WritePrivateProfileString(L"config", L"appToolbarHeight",
				std::to_wstring(appToolbarHeight).c_str(), iniPath.c_str());
		}
		else
		{
			appToolbarHeight = GetPrivateProfileInt(L"config", L"appToolbarHeight",
				appToolbarHeight, iniPath.c_str());
		}

		SetWindowPos(appToolbar, NULL, 0, 0, r.right - r.left,
			hideAppToolbar ? 0 : appToolbarHeight, SWP_NOMOVE | SWP_NOZORDER);
	}

	void hideIdentityPanelChanged()
	{
		WritePrivateProfileString(L"config", L"hideIdentityPanel", hideIdentityPanel ? L"1" : L"0",
			iniPath.c_str());

		HWND identityPanel = FindWindowEx(hWnd, NULL, CLS_IDENTITY_PANEL.c_str(), nullptr);
		if (!identityPanel)
			return;

		RECT r;
		GetWindowRect(identityPanel, &r);

		if (hideIdentityPanel)
		{
			identityPanelHeight = r.bottom - r.top;
			WritePrivateProfileString(L"config", L"identityPanelHeight",
				std::to_wstring(identityPanelHeight).c_str(), iniPath.c_str());
		}
		else
		{
			identityPanelHeight = GetPrivateProfileInt(L"config", L"identityPanelHeight",
				identityPanelHeight, iniPath.c_str());
		}

		SetWindowPos(identityPanel, NULL, 0, 0, r.right - r.left,
			hideIdentityPanel ? 0 : identityPanelHeight, SWP_NOMOVE | SWP_NOZORDER);
	}

	void languageIdChanged()
	{
		WritePrivateProfileString(L"config", L"languageId", std::to_wstring(languageId).c_str(),
			iniPath.c_str());
	}

	HMENU GetSkypeMenuAttachTo()
	{
		HMENU hMenuItem = NULL;
		MENUITEMINFO info;

		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_ID; // Get only menu item ID

		int count = GetMenuItemCount(hMenu);

		// Assume that the "Options" menu we want to attach our menu to is the second-to-last item,
		// before the final "Help" menu. Not exactly reliable, but not any worse than hardcoding
		// the menu ID and less likely to be broken by future Skype updates.
		hMenuItem = GetSubMenu(hMenu, count - 2);

		return hMenuItem;
	}

	LRESULT CALLBACK newWndProc(HWND _hwnd, UINT _message, WPARAM _wParam, LPARAM _lParam)
	{
		switch (_message)
		{
		case WM_ERASEBKGND:
		{
			// This message happens to be sent when the user changes Skype's layout. We can use
			// this to check whether we need to recreate our custom menu.
			HMENU hCurrent = GetMenu(_hwnd);
			if (hCurrent != hMenu)
			{
				createMenus(hCurrent);
				hMenu = hCurrent;
				// We can't attach our menu into Skype submenu because it will refill menu 
				// content for non english UI language on WM_INITMENUPOPUP message.
				hSkypeMenuAttachTo = GetSkypeMenuAttachTo();

				// Fallback, in case the Skype devs mess something up in the future.
				if (!hSkypeMenuAttachTo)
				{
					WCHAR buffer[200];
					LoadStringLang(IDS_MAINMENU, (LPTSTR)&buffer, sizeof(buffer));
					AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hUtilMenu, buffer);
				}
			}
			break;
		}
		case WM_INITMENUPOPUP:
			if ((HMENU)_wParam == hUtilMenu)
				updateUtilMenu();
			else if ((HMENU)_wParam == hLayoutMenu)
				updateLayoutMenu();
			else if ((HMENU)_wParam == hPosMenu)
				updatePosMenu();
			else if ((HMENU)_wParam == hDisplayMenu)
				updateDisplayMenu();
			else if ((HMENU)_wParam == hLanguageMenu)
				updateLanguageMenu();
			else if ((HMENU)_wParam == hSkypeMenuAttachTo)
			{
				// Let Skype dispatch message
				LRESULT r = CallWindowProc((WNDPROC) oldWndProc, _hwnd, _message, _wParam,
					_lParam);

				// and now attach our submenu
				attachMenu();

				return r;
			}
			break;
		case WM_COMMAND:
			if (_wParam == SKYPE_MENU_ID_VIEW_SPLIT)
			{
				// Let Skype dispatch message
				LRESULT r = CallWindowProc((WNDPROC)oldWndProc, _hwnd, _message, _wParam,
					_lParam);

				static auto timerProc =
					[](HWND _hwnd, UINT _uMsg, UINT_PTR _idEvent, DWORD _dwTime)
					{
						KillTimer(_hwnd, TIMER_ID_VIEW_CHANGED);
						hideAdsChanged();
					};
				// Dirty hack to force the layout to update after switching to split view.
				SetTimer(_hwnd, TIMER_ID_VIEW_CHANGED, viewChangeUpdateDelay, timerProc);

				return r;
			}
			if (_wParam == ID_ENABLE_CHAT_FORMAT)
			{
				enableChatFormat = !enableChatFormat;
				chatFormatChanged();
			}
			else if (_wParam == ID_HIDE_ADS)
			{
				hideAds = !hideAds;
				hideAdsChanged();
			}
			else if (_wParam == ID_HIDE_APP_TOOLBAR)
			{
				hideAppToolbar = !hideAppToolbar;
				hideAppToolbarChanged();
				forceLayoutUpdate(_hwnd);
			}
			else if (_wParam == ID_HIDE_IDENTITY_PANEL)
			{
				hideIdentityPanel = !hideIdentityPanel;
				hideIdentityPanelChanged();
				forceLayoutUpdate(_hwnd);
			}
			else if (_wParam == ID_SHOW_HELP)
			{
				ShellExecute(NULL, L"open",
					L"https://github.com/dlehn/Skype-Utility-Project#what-do-those-options-do",
					nullptr, nullptr, SW_SHOWNORMAL);
			}
			else if (_wParam == ID_SHOW_UPDATES)
			{
				ShellExecute(NULL, L"open",
					L"https://github.com/dlehn/Skype-Utility-Project/releases", nullptr, nullptr,
					SW_SHOWNORMAL);
			}
			else if (_wParam == ID_SHOW_CREDITS_DAVE)
			{
				ShellExecute(NULL, L"open", L"http://wordofdave.net/", nullptr, nullptr,
					SW_SHOWNORMAL);
			}
			else if (_wParam == ID_SHOW_CREDITS_MOE)
			{
				ShellExecute(NULL, L"open", L"http://kretzmoritz.wordpress.com/", nullptr,
					nullptr, SW_SHOWNORMAL);
			}
			else if (_wParam == ID_SHOW_CREDITS_NDS)
			{
				ShellExecute(NULL, L"open", L"https://github.com/nestdimon", nullptr,
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
			else if (_wParam >= ID_LANGUAGE_BASEID 
				&& _wParam <= ID_LANGUAGE_BASEID + 0xFF)
			{
				languageId = _wParam - ID_LANGUAGE_BASEID;

				// We need to recreate our menu with new language strings and redraw it
				HMENU hCurrent = GetMenu(_hwnd);
				if (hCurrent != hMenu)
					hMenu = hCurrent;
				DeleteMenu(hMenu, (UINT)hUtilMenu, MF_BYCOMMAND);
				createMenus(hCurrent);
				DrawMenuBar(_hwnd);
				languageIdChanged();
				break;
			}
			break;
		}

		return CallWindowProc((WNDPROC)oldWndProc, _hwnd, _message, _wParam, _lParam);
	}

	void APIENTRY notificationHook(HWINEVENTHOOK _hWinEventHook, DWORD _event, HWND _hwnd,
		LONG _idObject, LONG _idChild, DWORD _idEventThread, DWORD _dwmsEventTime)
	{
		wchar_t className[MAX_CLASS_NAME] = {0};
		GetClassName(_hwnd, className, MAX_CLASS_NAME);

		if (className == CLS_TRAY_ALERT)
		{
			RECT rect;
			if (!GetWindowRect(_hwnd, &rect))
				return;

			POINT pos = calcWindowPos(rect);
			MoveWindow(_hwnd, pos.x, pos.y, rect.right - rect.left, rect.bottom - rect.top,
				TRUE);

			return;
		}
		else if (className == CLS_CHAT_BANNER)
		{
			if (hideAds)
				SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			return;
		}

		switch (_event)
		{
		case EVENT_OBJECT_SHOW:
		{
			if (className == CLS_CHAT_RICH_EDIT)
			{
				commandHandler.executePendingCommands(_hwnd);
			}
			else if (!allowHideIdentityPanel && className == CLS_MAIN_FORM)
			{
				allowHideIdentityPanel = true;
				// Apply Hide Identity Panel option if it is enabled.
				if (hideIdentityPanel)
				{
					HWND hwnd = findVisibleChild(_hwnd, CLS_IDENTITY_PANEL.c_str(), nullptr);
					if (hwnd)
					{
						RECT r;
						GetWindowRect(hwnd, &r);
						SetWindowPos(hwnd, NULL, 0, 0, r.right - r.left, 0,
							SWP_NOMOVE | SWP_NOZORDER);
					}
				}
			}
			else if (hideAds && className == CLS_CONVERSATION_FORM)
			{
				// Required to force banners to be hidden when using split window mode in Skype
				// versions 7.1.60.105 and up.
				HWND hwnd = FindWindowEx(_hwnd, NULL, CLS_CHAT_BANNER.c_str(), nullptr);
				if (hwnd)
				{
					SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
			break;
		}
		case EVENT_OBJECT_LOCATIONCHANGE:
		{
			if ((hideAppToolbar && className == CLS_APP_TOOLBAR)
				|| (allowHideIdentityPanel && hideIdentityPanel
				&& className == CLS_IDENTITY_PANEL))
			{
				RECT r;
				GetWindowRect(_hwnd, &r);
				SetWindowPos(_hwnd, NULL, 0, 0, r.right - r.left, 0, SWP_NOMOVE | SWP_NOZORDER);
			}
			break;
		}
		}
	}

	DWORD WINAPI hook()
	{
		wchar_t buffer[MAX_PATH];

		DWORD procId = GetCurrentProcessId();
		SetWinEventHook(EVENT_OBJECT_SHOW,
			EVENT_OBJECT_SHOW, hInst,
			notificationHook, procId, NULL,
			WINEVENT_INCONTEXT | WINEVENT_SKIPOWNTHREAD);
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
		hideAds = GetPrivateProfileInt(L"config", L"hideAds", 0,
			iniPath.c_str()) != 0;
		hideAppToolbar = GetPrivateProfileInt(L"config", L"hideAppToolbar", 0,
			iniPath.c_str()) != 0;
		hideIdentityPanel = GetPrivateProfileInt(L"config", L"hideIdentityPanel", 0,
			iniPath.c_str()) != 0;
		languageId = GetPrivateProfileInt(L"config", L"languageId", LANG_DEFAULT,
			iniPath.c_str());
		notifCorner = (ScreenCorner)GetPrivateProfileInt(L"config", L"notifCorner", BottomRight,
			iniPath.c_str());
		GetPrivateProfileString(L"config", L"notifDisplay", L"", buffer, MAX_PATH,
			iniPath.c_str());
		notifDisplay = buffer;
		notifOffsetX = GetPrivateProfileInt(L"config", L"notifOffsetX", 0, iniPath.c_str());
		notifOffsetY = GetPrivateProfileInt(L"config", L"notifOffsetY", 0, iniPath.c_str());
		viewChangeUpdateDelay = GetPrivateProfileInt(L"config", L"viewChangeUpdateDelay", 500,
			iniPath.c_str());

		while (hWnd == NULL)
			hWnd = findWindowInProcess(L"tSkMainForm", nullptr);

		oldWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)newWndProc);

		commandHandler.setMainForm(hWnd);

		hideAdsChanged();

		// At Skype startup value of Lib/Conversation/EnableWiki is 1,
		// so no need to update if user has disabled "Disable Chat Formatting" option
		if (!enableChatFormat)
			chatFormatChanged();

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SUP::hook, nullptr, 0, nullptr);
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