#include "sup_window_util.h"

#include "sup_constants.h"

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

	void forceLayoutUpdate(HWND _hwnd)
	{
		WINDOWPOS pos;
		SendMessage(_hwnd, WM_WINDOWPOSCHANGED, NULL, (LPARAM)&pos);
	}
}