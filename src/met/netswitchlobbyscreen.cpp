#include "met/netswitchlobbyscreen.h"

#include <cstring>

#include "met/chatpanel.h"
#include "met/metagame.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

NetSwitchLobbyScreen::NetSwitchLobbyScreen(DataArray *pData) : FreqScreen(pData) {
}

bool NetSwitchLobbyScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoinChatroomResultMsgType) {
        return HandleJoinResult(static_cast<JoinChatroomResultMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetSwitchLobbyScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheUI.FindPanel("fn_main", false)->SetFocus(nullptr, kPadNone);
        TheNetLobby->JoinChatroom(this, mChatroomId);
    }
    return false;
}

bool NetSwitchLobbyScreen::HandleJoinResult(JoinChatroomResultMsg *pMsg) {
    if (pMsg->mResult == 0) {
        dynamic_cast<ChatPanel *>(TheUI.FindPanel("fn_main_c", false))->mMessages.clear();
        TheMetagame.mChatroom = pMsg->mChatroom;
        TheUI.GotoScreen("net_switch_lobby_success");
    } else {
        TheUI.GotoScreen("net_switch_lobby_error");
    }
    return false;
}
