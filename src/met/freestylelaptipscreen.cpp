#include "met/freestylelaptipscreen.h"

#include "met/metagame.h"
#include "os/joypad.h"

FreestyleLapTipScreen::FreestyleLapTipScreen(DataArray *pData) : FreqScreen(pData) {
}

bool FreestyleLapTipScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool FreestyleLapTipScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadCross && pMsg->mPressed) {
        TheMetagame.AdvanceUnlocks();
    }
    return FreqScreen::HandleJoypad(pMsg);
}
