#ifndef SUP_CHAT_COMMANDS_H
#define SUP_CHAT_COMMANDS_H

#include <Windows.h>
#include <mutex>
#include <vector>
#include <string>

namespace SUP
{
	// Thread safe command handler.
	class ChatCommandHandler
	{
	public:
		ChatCommandHandler(HWND _mainForm = NULL);

		void setMainForm(HWND _mainForm);

		// Queue the command _cmd for execution once a chat control is available.
		// _groupIdent serves to e.g. remove a certain group of commands if they were to
		// be overwritten by a new command.
		// If _unique is true, all pending commands using _groupIdent will be removed before
		// adding _cmd.
		void queueCommand(const std::wstring& _cmd, const std::wstring& _groupIdent,
			bool _unique = false);

		// Manually remove all pending commands registered using _groupIdent.
		void removePendingCommandGroup(const std::wstring& _groupIdent);

		// Execute any pending chat commands using the given _chatRichEdit.
		void executePendingCommands(HWND _chatRichEdit);

		// Find a suitable chat rich edit to post commands to.
		HWND findChatRichEdit();

	private:
		// First is command, second is group identifier.
		std::vector<std::pair<std::wstring, std::wstring>> m_PendingCommands;
		std::mutex m_AccessMutex;
		HWND m_MainForm;
	};
}

#endif