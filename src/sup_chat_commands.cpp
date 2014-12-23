#include "sup_chat_commands.h"

#include "sup_constants.h"
#include "sup_window_util.h"

namespace SUP
{
	ChatCommandHandler::ChatCommandHandler(HWND _mainForm)
		: m_MainForm(_mainForm)
	{

	}

	void ChatCommandHandler::setMainForm(HWND _mainForm)
	{
		m_MainForm = _mainForm;
	}

	void ChatCommandHandler::queueCommand(const std::wstring& _cmd,
		const std::wstring& _groupIdent, bool _unique)
	{
		if (_unique)
			removePendingCommandGroup(_groupIdent);

		m_AccessMutex.lock();
		m_PendingCommands.push_back(std::make_pair(_cmd, _groupIdent));
		m_AccessMutex.unlock();

		executePendingCommands(findChatRichEdit());
	}

	void ChatCommandHandler::removePendingCommandGroup(const std::wstring& _groupIdent)
	{
		m_AccessMutex.lock();

		for (int i = (int)m_PendingCommands.size() - 1; i >= 0; i--)
		{
			if (m_PendingCommands[i].second == _groupIdent)
				m_PendingCommands.erase(m_PendingCommands.begin() + i);
		}

		m_AccessMutex.unlock();
	}

	void ChatCommandHandler::executePendingCommands(HWND _chatRichEdit)
	{
		if (!_chatRichEdit || m_PendingCommands.empty())
			return;

		if (!m_AccessMutex.try_lock())
			return;

		wchar_t buffer[8192];
		GetWindowText(_chatRichEdit, buffer, 8192);

		for (const auto& cmd : m_PendingCommands)
		{
			SetWindowText(_chatRichEdit, cmd.first.c_str());

			SendMessage(_chatRichEdit, WM_KEYDOWN, VK_RETURN, 0);
			SendMessage(_chatRichEdit, WM_KEYUP, VK_RETURN, 0);
		}

		m_PendingCommands.clear();

		SetWindowText(_chatRichEdit, buffer);
		SendMessage(_chatRichEdit, WM_SETTEXT, 0, (LPARAM)buffer);

		m_AccessMutex.unlock();
	}

	HWND ChatCommandHandler::findChatRichEdit()
	{
		HWND hWndChat = findWindowInProcess(CLS_CONVERSATION_FORM.c_str(), nullptr);

		if (hWndChat == NULL)
		{
			// Combined View
			hWndChat = findVisibleChild(m_MainForm, CLS_CONVERSATION_FORM.c_str(), nullptr);

			if (hWndChat == NULL)
			{
				return NULL;
			}
		}

		HWND hWndControl = findVisibleChild(hWndChat, CLS_CHAT_ENTRY.c_str(), nullptr);

		if (hWndControl == NULL)
		{
			return NULL;
		}

		HWND hWndText = findVisibleChild(hWndControl, CLS_CHAT_RICH_EDIT.c_str(), nullptr);
		if (hWndText == NULL)
		{
			return NULL;
		}

		return hWndText;
	}
}