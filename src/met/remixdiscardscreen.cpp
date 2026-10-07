#include "met/remixdiscardscreen.h"

#include "game/gamedb.h"
#include "met/metagame.h"
#include "met/netjoinlpadscreen.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kLocalSaveFormat[] = "m_r_end_remix_%d";

// IsGuest() reports this value for the host.
constexpr int kHostIndex = 0;

} // namespace

RemixDiscardScreen::RemixDiscardScreen(DataArray *pData) : FreqScreen(pData) {
}

bool RemixDiscardScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RemixDiscardScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    String button(pMsg->mComponent->mName);
    String next;
    const bool bYes = button == "yes";
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        next = bYes ? "s_r_mode" : "s_r_end_remix";
    } else if (TheGameDb->mCommunity == GameDb::kCommunityLocal) {
        if (!bYes) {
            next = FormatString(kLocalSaveFormat, mPlayer);
        } else if (mPlayer == TheGameDb->GetNumPlayers()) {
            next = "m_r_mode";
        } else {
            next = FormatString(kLocalSaveFormat, mPlayer + 1);
        }
    } else if (!bYes) {
        next = "net_end_remix";
    } else if (TheNetLaunchpad == nullptr) {
        if (TheMetagame.mNetScreenPending != 0) {
            next = TheMetagame.mNetScreen.c_str();
        } else {
            NetJoinLPadScreen::ShowLaunchpadLost();
        }
    } else {
        next = TheNetLaunchpad->IsGuest() == kHostIndex ? "fn_h_lpad" : "fn_g_lpad";
    }
    TheUI.GotoScreen(next.c_str()); // Yes, the binary also goes to an empty name.
    return true;
}
