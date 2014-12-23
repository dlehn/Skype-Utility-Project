#ifndef SUP_WINDOW_UTIL_H
#define SUP_WINDOW_UTIL_H

#include <Windows.h>

namespace SUP
{
	// Find the next window matching _wndClass and _wndTitle in the current process.
	// If _prev is specified, will continue the search after the given window.
	HWND findWindowInProcess(const wchar_t* _wndClass, const wchar_t* _wndTitle,
		HWND _prev = NULL);

	// Returns the first visible child window of _parent matching the criteria. If no visible
	// child windows are available, the first found child window will be returned. If no
	// matching child windows were found at all, returns NULL.
	HWND findVisibleChild(HWND _parent, const wchar_t* _wndClass, const wchar_t* _wndTitle);
}

#endif