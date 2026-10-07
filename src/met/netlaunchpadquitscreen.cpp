#include "met/netlaunchpadquitscreen.h"

#include <cstring>

#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kLobbyTransition[] = "netlaunchpad2netlobby";

} // namespace

NetLaunchpadQuitScreen::NetLaunchpadQuitScreen(DataArray *pData) : FreqScreen(pData) {
}

bool NetLaunchpadQuitScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLaunchpadAbortedMsgType) {
        return HandleAborted(static_cast<LaunchpadAbortedMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetLaunchpadQuitScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->mScreen->mName, mName) == 0) {
        if (TheNetLaunchpad != nullptr) {
            TheNetLaunchpad->Cancel(this);
        } else {
            TheUI.GotoScreen(kLobbyTransition);
        }
    }
    return false;
}

bool NetLaunchpadQuitScreen::HandleAborted([[maybe_unused]] LaunchpadAbortedMsg *pMsg) {
    TheUI.GotoScreen(kLobbyTransition);
    return false;
}
