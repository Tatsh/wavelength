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
    if (pMsg->GetPressed() && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->GetButton() == kPadCross && pMsg->GetPressed()) {
        TheMetagame.AdvanceUnlocks();
    }
    return FreqScreen::HandleJoypad(pMsg);
}
