#include "met/netjoinlpadscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/chatpanel.h"
#include "met/dialogpanel.h"
#include "met/metagame.h"
#include "met/transitionerrorscreen.h"
#include "netflow/joinpadrt.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uimanager.h"

namespace {

constexpr char kErrorScreen[] = "lpad_error";
constexpr char kLobbyErrorScreen[] = "lobby_error";
constexpr int kNoLaunchpad = -1;

// The reasons the network layer gives for a refused join.
enum LaunchpadJoinError {
    kJoinErrorIncompatibleHost = -70,
    kJoinErrorFull = -69,
    kJoinErrorInProgress = -68,
    kJoinErrorDenied = -67,
    kJoinErrorIncompatible = -66,
};

const char *JoinErrorToken(int nReason) {
    switch (nReason) {
    case kJoinErrorFull:
        return "net_lpad_full";
    case kJoinErrorInProgress:
        return "net_lpad_in_progress";
    case kJoinErrorDenied:
        return "net_lpad_denied";
    case kJoinErrorIncompatibleHost:
        return "net_lpad_incompatible_host";
    case kJoinErrorIncompatible:
        return "net_lpad_incompatible";
    default:
        return "net_join_lpad_error";
    }
}

bool IsErrorScreen(const UIScreen *pScreen) {
    return strcmp(pScreen->mName, kLobbyErrorScreen) == 0 ||
           strcmp(pScreen->mName, kErrorScreen) == 0;
}

} // namespace

NetJoinLPadScreen::NetJoinLPadScreen(DataArray *pData) : FreqScreen(pData) {
    mLaunchpadWorld = kNoLaunchpad;
    mLaunchpadId = kNoLaunchpad;
    mCancelling = 0;
}

void NetJoinLPadScreen::ShowLaunchpadLost() {
    UIScreen *pCurrent = TheUI.mCurrentScreen;
    if (pCurrent != nullptr && IsErrorScreen(pCurrent)) {
        return;
    }
    // The binary reads mNextScreen through the null pointer when no screen shows.
    UIScreen *pNext = pCurrent != nullptr ? pCurrent->mNextScreen : nullptr;
    if (pNext != nullptr && IsErrorScreen(pNext)) {
        return;
    }
    String text(TheLocale.Localize("net_lpad_lost_error_msg", true));
    auto *pScreen = dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
    pScreen->mMessage = text.c_str();
    TheUI.GotoScreen(pScreen);
}

bool NetJoinLPadScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoinLaunchpadSuccessMsgType) {
        return HandleSuccess(static_cast<JoinLaunchpadSuccessMsg *>(pMsg));
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

bool NetJoinLPadScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->mScreen->mName, mName) == 0) {
        mCancelling = 0;
        mReturnScreen = pMsg->mPrevScreen->mName;
        JoinpadRT::Create(this, &TheMetagame, mLaunchpadId, mLaunchpadWorld);
    }
    return false;
}

bool NetJoinLPadScreen::HandleSuccess(JoinLaunchpadSuccessMsg *pMsg) {
    if (mCancelling != 0) {
        return false;
    }
    TheGameDb->SetGameParams(&pMsg->mParams);
    dynamic_cast<ChatPanel *>(TheUI.FindPanel("fn_h_lpad_c", false))->mMessages.clear();
    TheUI.GotoScreen("netlobby2netlaunchpad_guest");
    return false;
}

bool NetJoinLPadScreen::HandleAborted(LaunchpadAbortedMsg *pMsg) {
    if (pMsg->mReason == 0) {
        TheUI.GotoScreen(mReturnScreen.c_str());
        return false;
    }
    String text(TheLocale.Localize(FormatString("%s_msg", JoinErrorToken(pMsg->mReason)), true));
    auto *pScreen = dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
    pScreen->mMessage = text.c_str();
    pScreen->ClearTransitions();
    pScreen->AddTransition("ok", kPadNone, mReturnScreen.c_str());
    TheUI.GotoScreen(pScreen);
    return false;
}

bool NetJoinLPadScreen::HandleJoypad(JoypadInputMsg *pMsg) {
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
            ->SetText(TheLocale.Localize("cancel_join_attempt", true));
        TheNetLaunchpad->Cancel(this);
    } else {
        TheUI.GotoScreen(mReturnScreen.c_str());
    }
    return false;
}
