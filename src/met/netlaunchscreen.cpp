#include "met/netlaunchscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/netjoinlpadscreen.h"
#include "netflow/netlaunchpad.h"
#include "ui/uimanager.h"

NetLaunchScreen::NetLaunchScreen(DataArray *pData) : FreqScreen(pData) {
}

bool NetLaunchScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetLaunchScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (TheNetLaunchpad != nullptr && strcmp(pMsg->mScreen->mName, mName) == 0) {
        (void)TheNetLaunchpad->Launch(); // Yes, the binary discards this call's result.
        if (TheGameDb->mLoadRemix != 0) {
            TheUI.GotoScreen("net_share_remix");
        }
    } else {
        NetJoinLPadScreen::ShowLaunchpadLost();
    }
    return false;
}
