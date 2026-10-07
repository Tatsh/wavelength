#include "met/xproceedscreen.h"

#include "os/joypad.h"
#include "synth/fxmidi.h"

bool XProceedScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool XProceedScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    const bool bHandled = FreqScreen::HandleJoypad(pMsg);
    if (!bHandled && pMsg->mButton == kPadCross && pMsg->mPressed != 0) {
        FxMidi::PlayMenuSelect();
    }
    return bHandled;
}
