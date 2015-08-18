#include "sup_window_util.h"

#include "sup_constants.h"

#pragma comment(lib, "oleacc.lib")

namespace SUP
{
	HWND findWindowInProcess(const wchar_t* _wndClass, const wchar_t* _wndTitle, HWND _prev)
	{
		DWORD procId = GetCurrentProcessId();
		DWORD wndProcId = NULL;
		HWND result = _prev;
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

	HWND findVisibleChild(HWND _parent, const wchar_t* _wndClass, const wchar_t* _wndTitle, HWND _prev)
	{
		HWND resultFirst = FindWindowEx(_parent, _prev, _wndClass, _wndTitle);
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

	void forceLayoutUpdate(HWND _hwnd)
	{
		RECT r;
		GetWindowRect(_hwnd, &r);
		SetWindowPos(_hwnd, NULL, 0, 0, r.right - r.left + 1, r.bottom - r.top + 1,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		SetWindowPos(_hwnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	CComPtr<IHTMLDocument2> getHTMLDocumentFromIEServer(HWND _hwnd)
	{
		UINT msg = RegisterWindowMessage(_T("WM_HTML_GETOBJECT"));
		LRESULT result = SendMessage(_hwnd, msg, 0, 0);

		CComPtr<IHTMLDocument2> spDoc;
		ObjectFromLresult(result, IID_IHTMLDocument2, 0, (void**)&spDoc);
		return spDoc;
	}

	HMENU getMenuByID(HMENU _hParent, UINT _id)
	{
		HMENU hMenuItem = NULL;
		MENUITEMINFO info;

		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_ID; // Get only menu item ID

		int count = GetMenuItemCount(_hParent);

		// Yes, there is the only one stupid way to get menu ID by its HMENU
		for (int i = 0; i < count; i++)
		{
			if (GetMenuItemInfo(_hParent, i, true, &info) && info.wID == _id)
			{
				hMenuItem = GetSubMenu(_hParent, i);
				break;
			}
		}

		return hMenuItem;
	}
}