#include "met/netlobbydisconnect.h"

#include <cstring>

#include "netflow/netlobby.h"
#include "ui/uimanager.h"

namespace {

constexpr char kAskScreen[] = "net_disconnect_lobby_ask";
constexpr char kWelcomeToPortalScreen[] = "netwelcome2netportal";
#ifdef VIDEO_STANDARD_PAL
// The European release disconnects from the network in place of returning to the portal.
constexpr char kPortalScreen[] = "disconnect_internet";

// The screen that disconnects after a DNAS failure, and the screen it goes to.
constexpr char kDnasScreen[] = "net_disconnect_lobby_dnas";
constexpr char kDisconnectScreen[] = "disconnect_internet";
#else
constexpr char kPortalScreen[] = "net_portal";
#endif

} // namespace

NetLobbyDisconnect::NetLobbyDisconnect(DataArray *pData) : FreqScreen(pData), mAsked(0) {
}

bool NetLobbyDisconnect::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLobbyDisconnectResultMsgType) {
        return HandleDisconnectResult(static_cast<LobbyDisconnectResultMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetLobbyDisconnect::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        mAsked = std::strcmp(pMsg->mPrevScreen->mName, kAskScreen) == 0;
        TheNetLobby->Disconnect(this);
    }
    return false;
}

bool NetLobbyDisconnect::HandleDisconnectResult([[maybe_unused]] LobbyDisconnectResultMsg *pMsg) {
#ifdef VIDEO_STANDARD_PAL
    if (strcmp(mName, kDnasScreen) == 0) {
        TheUI.GotoScreen(kDisconnectScreen);
        return false;
    }
#endif
    TheUI.GotoScreen(mAsked != 0 ? kWelcomeToPortalScreen : kPortalScreen);
    return false;
}
