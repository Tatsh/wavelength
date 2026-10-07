#include "met/marketingscreen.h"

#include "os/joypad.h"
#include "os/system.h"

namespace {

// mEndTime while no hold runs.
constexpr float kNoHold = -1.0f;

// Subtracted from the system time to end the hold at the next poll.
constexpr float kHoldEndMargin = 1.0f;

} // namespace

bool MarketingScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (UIScreen::mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton != kPadCross || mEndTime == kNoHold) {
        return false;
    }
    mEndTime = SystemMs() - kHoldEndMargin;
    return false;
}

bool MarketingScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return IntroScreen::DispatchPriv(pMsg);
}
