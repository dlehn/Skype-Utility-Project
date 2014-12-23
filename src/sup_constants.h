#ifndef SUP_CONSTANTS_H
#define SUP_CONSTANTS_H

#include <string>

namespace SUP
{
	// Class name constants
	extern const std::wstring CLS_MAIN_FORM;
	extern const std::wstring CLS_CONVERSATION_FORM;
	extern const std::wstring CLS_CHAT_ENTRY;
	extern const std::wstring CLS_CHAT_RICH_EDIT;
	extern const std::wstring CLS_CHAT_BANNER;
	extern const std::wstring CLS_TRAY_ALERT;
}

// Menu IDs
#define ID_ENABLE_CHAT_FORMAT		2000
#define ID_HIDE_ADS					2001
#define ID_SET_NOTIFICATION_DISPLAY	2100
#define ID_SET_NOTIFICATION_POS		2200
#define ID_SHOW_UPDATES				2300
#define ID_SHOW_CREDITS_MOE			2301
#define ID_SHOW_CREDITS_DAVE		2302

// Version String
#define SUP_VERSION L"1.1.0"

#endif