#include "met/netnewchatroomscreen.h"

#include <cstring>

#include "met/chatpanel.h"
#include "met/metagame.h"
#include "met/setupnamescreen.h"
#include "met/transitionerrorscreen.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kChatPanel[] = "fn_main_c";
constexpr char kMainPanel[] = "fn_main";
constexpr char kSuccessScreen[] = "net_create_lobby_success";
constexpr char kSetupScreen[] = "fn_new_lob";
constexpr char kErrorScreen[] = "net_create_lobby_error";
constexpr char kNameExistsToken[] = "chatroom_name_exists_error";
constexpr char kVulgarNameToken[] = "chatroom_text_vulgar_error";
constexpr char kCreateErrorToken[] = "net_create_lobby_error_dlg";

} // namespace

NetNewChatroomScreen::NetNewChatroomScreen(DataArray *pData) : FreqScreen(pData) {
}

bool NetNewChatroomScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoinChatroomResultMsgType) {
        return HandleJoinChatroomResult(static_cast<JoinChatroomResultMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetNewChatroomScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        dynamic_cast<ChatPanel *>(TheUI.FindPanel(kChatPanel, false))->mMessages.clear();
        TheUI.FindPanel(kMainPanel, false)->SetFocus(nullptr, kPadNone);
        TheNetLobby->CreateChatroom(this, mChatroomName);
    }
    return false;
}

bool NetNewChatroomScreen::HandleJoinChatroomResult(JoinChatroomResultMsg *pMsg) {
    if (pMsg->mResult == JoinChatroomResultMsg::kResultSuccess) {
        TheMetagame.mChatroom = pMsg->mChatroom;
        TheUI.GotoScreen(kSuccessScreen);
        return false;
    }

    dynamic_cast<SetupNameScreen *>(TheUI.FindScreen(kSetupScreen, false))
        ->SetText(mChatroomName.c_str());
    const char *pszToken;
    if (pMsg->mResult == JoinChatroomResultMsg::kResultNameExists) {
        pszToken = kNameExistsToken;
    } else if (pMsg->mResult == JoinChatroomResultMsg::kResultVulgarName) {
        pszToken = kVulgarNameToken;
    } else {
        pszToken = kCreateErrorToken;
    }
    TransitionErrorScreen *pError =
        dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
    pError->mMessage = TheLocale.Localize(pszToken, true);
    TheUI.GotoScreen(pError);
    return false;
}
