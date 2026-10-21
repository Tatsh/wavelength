#include "met/unlockscreen.h"

#include "met/metagame.h"
#include "os/joypad.h"
#include "synth/fxmidi.h"

UnlockScreen::UnlockScreen(DataArray *pData) : XProceedScreen(pData) {
}

bool UnlockScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool UnlockScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetPressed() && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->GetButton() == kPadCross && pMsg->GetPressed()) {
        FxMidi::PlayMenuSelect();
        TheMetagame.AdvanceUnlocks();
    }
    return FreqScreen::HandleJoypad(pMsg);
}
