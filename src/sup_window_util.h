#ifndef SUP_WINDOW_UTIL_H
#define SUP_WINDOW_UTIL_H

#include <Windows.h>
#include <mshtml.h>
#include <atlbase.h>
#include <oleacc.h>

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

	// Force a window to update its layout.
	void forceLayoutUpdate(HWND _hwnd);

	// Fetches an IHTMLDocument2 interface from a HWND of class Internet Explorer_Server.
	// This assumes that _hwnd is a valid handle to an Internet Explorer_Server window.
	CComPtr<IHTMLDocument2> getHTMLDocumentFromIEServer(HWND _hwnd);

	// Retrieve a menu handle by its ID.
	HMENU getMenuByID(HMENU _hParent, UINT _id);
}

#endif