#include "met/netlobbyconnect.h"

#include <cstring>

#include "met/transitionerrorscreen.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kLoginScreen[] = "net_login";
constexpr char kInetErrorScreen[] = "inet_error";
constexpr char kLostInternetToken[] = "lost_internet_error_msg";
constexpr char kConnectErrorScreen[] = "net_connect_lobby_error";
constexpr char kDisconnectLobbyScreen[] = "net_disconnect_lobby";

} // namespace

NetLobbyConnect::NetLobbyConnect(DataArray *pData) : FreqScreen(pData) {
}

bool NetLobbyConnect::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLobbyConnectResultMsgType) {
        return HandleConnectResult(static_cast<LobbyConnectResultMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetLobbyConnect::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLobby->Connect(this);
    }
    return false;
}

bool NetLobbyConnect::HandleConnectResult(LobbyConnectResultMsg *pMsg) {
    if (pMsg->mResult == LobbyConnectResultMsg::kResultSuccess) {
        TheUI.GotoScreen(kLoginScreen);
    } else if (pMsg->mResult == LobbyConnectResultMsg::kResultLostInternet) {
        TransitionErrorScreen *pError =
            dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kInetErrorScreen, false));
        pError->mMessage = TheLocale.Localize(kLostInternetToken, true);
        TheUI.GotoScreen(pError);
    } else {
        TheUI.GotoScreen(kConnectErrorScreen);
    }
    return false;
}

bool NetLobbyConnect::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle) {
        TheUI.GotoScreen(kDisconnectLobbyScreen);
    }
    return false;
}
