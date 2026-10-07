#include "met/shareremixscreen.h"

#include "game/gamedb.h"
#include "met/dialogpanel.h"
#include "met/metagame.h"
#include "met/netjoinlpadscreen.h"
#include "met/saveremixscreen.h"
#include "netflow/netlaunchpad.h"
#include "os/debug.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kGuestLaunchpad[] = "fn_g_lpad";
constexpr char kHostLaunchpad[] = "fn_h_lpad";

// IsGuest() reports this value for the host.
constexpr int kHostIndex = 0;

constexpr float kPercent = 100.0f;

} // namespace

ShareRemixScreen::ShareRemixScreen(DataArray *pData) : FreqScreen(pData) {
    mPendingScreen = "";
}

bool ShareRemixScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nShareRemixProgressMsgType) {
        return HandleProgress(static_cast<ShareRemixProgressMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool ShareRemixScreen::HandleProgress(ShareRemixProgressMsg *pMsg) {
    if (!(static_cast<double>(pMsg->mProgress) >= 1.0)) {
        DebugPrint("ShareRemixProgressMsg:: progress\n");
        String text(FormatString(TheLocale.Localize("share_progress", true),
                                 static_cast<int>(pMsg->mProgress * kPercent)));
        dynamic_cast<DialogPanel *>(mFocusPanel)->SetText(text.c_str());
        return false;
    }

    DebugPrint("share done\n");
    String next;
    if (TheNetLaunchpad == nullptr) {
        DebugPrint("ShareRemixProgressMsg::launchpad died\n");
        if (TheMetagame.mNetScreenPending == 0) {
            NetJoinLPadScreen::ShowLaunchpadLost();
            return false;
        }
        next = TheMetagame.mNetScreen.c_str();
    } else if (pMsg->mReceived != 0 && TheNetLaunchpad->IsGuest() != kHostIndex) {
        auto *pSave = dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen("save_remix", false));
        TheDebug << "just received this remix " << *TheGameDb->GetRemixInfo() << "\n";
        if (TheGameDb->mRemixReadOnly != 0) {
            next = "net_save_read_only";
            pSave->mRemixName = TheGameDb->GetRemixInfo()->mName;
        } else {
            next = "net_end_remix";
        }
        pSave->SetStartScreen(kGuestLaunchpad);
        pSave->SetDoneScreen(kGuestLaunchpad);
    } else if (TheNetLaunchpad->IsGuest() == kHostIndex) {
        next = kHostLaunchpad;
    } else {
        next = kGuestLaunchpad;
    }

    if (TheUI.mCurrentScreen == this) {
        DebugPrint("ShareRemixProgressMsg::going to%s\n", next.c_str());
        TheUI.GotoScreen(next.c_str());
    } else {
        mPendingScreen = next;
    }
    return false;
}

bool ShareRemixScreen::HandleTransitionComplete([[maybe_unused]] UITransitionCompleteMsg *pMsg) {
    if (mPendingScreen != "") {
        TheUI.GotoScreen(mPendingScreen.c_str());
        mPendingScreen = "";
    }
    return false;
}
