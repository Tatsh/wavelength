#include "met/netlobbyconnect.h"

#include <cstring>

#include "met/transitionerrorscreen.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

#ifdef VIDEO_STANDARD_PAL
#include "netflow/dnastask.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtext.h"
#endif

namespace {

constexpr char kLoginScreen[] = "net_login";
constexpr char kInetErrorScreen[] = "inet_error";
constexpr char kLostInternetToken[] = "lost_internet_error_msg";
constexpr char kConnectErrorScreen[] = "net_connect_lobby_error";
constexpr char kDisconnectLobbyScreen[] = "net_disconnect_lobby";

#ifdef VIDEO_STANDARD_PAL
constexpr char kDNASMesh[] = "dnas.mesh";
constexpr char kDialogText[] = "dnas_net_connect_lobby.txt";
constexpr char kDNASToken[] = "dnas_net_connect_lobby";
constexpr char kDialogToken[] = "net_connect_lobby_dlg";
constexpr char kDNSErrorScreen[] = "net_dns_error";
constexpr char kDNSErrorToken[] = "net_dns_error_msg";
constexpr char kDNASErrorScreen[] = "net_dnas_error";
constexpr char kDNASErrorToken[] = "net_dnas_error_msg";
#endif

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
#ifdef VIDEO_STANDARD_PAL
    if (nType == g_nDNASStatusMsgType) {
        return HandleDNASStatus(static_cast<DNASStatusMsg *>(pMsg));
    }
#endif
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

#ifdef VIDEO_STANDARD_PAL
void NetLobbyConnect::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    ShowDNAS(0, 0);
}

void NetLobbyConnect::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
}

bool NetLobbyConnect::HandleDNASStatus(DNASStatusMsg *pMsg) {
    ShowDNAS(pMsg->mRunning, 1);
    return false;
}

void NetLobbyConnect::ShowDNAS(int nRunning, int nShow) {
    RndMesh *pMesh = dynamic_cast<RndMesh *>(TheManager.Find(kDNASMesh));
    RndText *pText = dynamic_cast<RndText *>(TheManager.Find(kDialogText));
    if (pMesh != nullptr) {
        pMesh->SetShowing(nRunning != 0 && nShow != 0);
    }
    if (pText != nullptr) {
        pText->SetShowing(nShow);
        if (nRunning != 0) {
            pText->SetText(TheLocale.Localize(kDNASToken, true));
        } else {
            pText->SetText(TheLocale.Localize(kDialogToken, true));
        }
    }
}
#endif

bool NetLobbyConnect::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->GetName(), mName) == 0) {
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
#ifdef VIDEO_STANDARD_PAL
    } else if (pMsg->mResult == LobbyConnectResultMsg::kResultLookupFailed) {
        TransitionErrorScreen *pError =
            dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kDNSErrorScreen, false));
        pError->mMessage = TheLocale.Localize(kDNSErrorToken, true);
        TheUI.GotoScreen(pError);
    } else if (pMsg->mResult == LobbyConnectResultMsg::kResultDNASFailed) {
        TransitionErrorScreen *pError =
            dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kDNASErrorScreen, false));
        int nSubCode;
        const char *pszError;
        const char *pszHint;
        DNASTask::GetError(&nSubCode, &pszError, &pszHint);
        const String error(TheLocale.Localize(pszError, true));
        const String hint(TheLocale.Localize(pszHint, true));
        pError->mMessage = FormatString(
            TheLocale.Localize(kDNASErrorToken, true), nSubCode, error.c_str(), hint.c_str());
        TheUI.GotoScreen(pError);
#endif
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
