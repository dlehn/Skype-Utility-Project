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
	extern const std::wstring CLS_APP_TOOLBAR;
	extern const std::wstring CLS_IDENTITY_PANEL;
}

// Menu IDs
#define ID_ENABLE_CHAT_FORMAT		2000

#define ID_HIDE_ADS					2100
#define ID_HIDE_APP_TOOLBAR			2101
#define ID_HIDE_IDENTITY_PANEL		2102

#define ID_SET_NOTIFICATION_DISPLAY	2200

#define ID_SET_NOTIFICATION_POS		2300

#define ID_SHOW_HELP				2400
#define ID_SHOW_UPDATES				2401
#define ID_SHOW_CREDITS_MOE			2402
#define ID_SHOW_CREDITS_DAVE		2403

#define ID_LANGUAGE_BASEID			3000
#define ID_LANGUAGE_GERMAN			ID_LANGUAGE_BASEID + LANG_GERMAN
#define ID_LANGUAGE_ENGLISH			ID_LANGUAGE_BASEID + LANG_ENGLISH
#define ID_LANGUAGE_RUSSIAN			ID_LANGUAGE_BASEID + LANG_RUSSIAN
#define LANG_DEFAULT				LANG_ENGLISH

// Skype mainmenu item ID attach to
// 0x45 Skype
// 0x46 Contacts
// 0x47 Call
// 0x48 Conversation
// 0x49 View
// 0x4A Tools
// 0x4B Help
#define SKYPE_MENU_ID_ATTACH_TO		0x4A

// Version String
#define SUP_VERSION L"1.3.3"

#endif