#include "met/nethostattemptscreen.h"

#include <cstring>

#include "met/chatpanel.h"
#include "met/dialogpanel.h"
#include "met/metagame.h"
#include "netflow/hostpadrt.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uimanager.h"

namespace {

constexpr char kHostingScreen[] = "fn_hosting";

} // namespace

NetHostAttemptScreen::NetHostAttemptScreen(DataArray *pData) : FreqScreen(pData), mCancelling(0) {
}

bool NetHostAttemptScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nHostLaunchpadSuccessMsgType) {
        return HandleSuccess(static_cast<HostLaunchpadSuccessMsg *>(pMsg));
    }
    if (nType == g_nLaunchpadAbortedMsgType) {
        return HandleAborted(static_cast<LaunchpadAbortedMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetHostAttemptScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->mScreen->mName, mName) == 0) {
        mCancelling = 0;
        HostpadRT::Create(this, &TheMetagame);
    }
    return false;
}

bool NetHostAttemptScreen::HandleSuccess([[maybe_unused]] HostLaunchpadSuccessMsg *pMsg) {
    dynamic_cast<ChatPanel *>(TheUI.FindPanel("fn_h_lpad_c", false))->mMessages.clear();
    if (mCancelling == 0) {
        TheUI.GotoScreen("netlobby2netlaunchpad_host");
    }
    return false;
}

bool NetHostAttemptScreen::HandleAborted(LaunchpadAbortedMsg *pMsg) {
    if (pMsg->mReason == 0) {
        TheUI.GotoScreen(kHostingScreen);
    } else {
        TheUI.GotoScreen("net_hosting_error");
    }
    return false;
}

bool NetHostAttemptScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton != kPadTriangle) {
        return false;
    }
    if (TheNetLaunchpad != nullptr) {
        mCancelling = 1;
        dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("dialog_05a.txt"))->SetShowing(false);
        dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("dialog_05b.txt"))->SetShowing(false);
        dynamic_cast<DialogPanel *>(mFocusPanel)
            ->SetText(TheLocale.Localize("cancel_host_attempt", true));
        TheNetLaunchpad->Cancel(this);
    } else {
        TheUI.GotoScreen(kHostingScreen);
    }
    return false;
}
