#include "met/introscreen.h"

#include "os/system.h"
#include "ui/uimanager.h"

namespace {

constexpr char kNextScreenKey[] = "next_screen";
constexpr char kHoldTimeKey[] = "hold_time";

// mEndTime while no hold runs.
constexpr float kNoHold = -1.0f;

constexpr float kDefaultHoldMs = 1000.0f;

} // namespace

IntroScreen::IntroScreen(DataArray *pData) : UIScreen(pData) {
    mEndTime = kNoHold;
    mHoldMs = kDefaultHoldMs;
    pData->FindSymbol(kNextScreenKey, &mNextScreen, true);
    pData->FindFloat(kHoldTimeKey, &mHoldMs, false);
}

void IntroScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (0.0f < mEndTime && mEndTime < SystemMs()) {
        TheUI.GotoScreen(mNextScreen);
        mEndTime = kNoHold;
    }
}

bool IntroScreen::HandleTransitionComplete([[maybe_unused]] UITransitionCompleteMsg *pMsg) {
    mEndTime = SystemMs() + mHoldMs;
    return false;
}

bool IntroScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return UIScreen::DispatchPriv(pMsg);
}
