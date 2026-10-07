#include "met/keyboardkey.h"

#include "os/joypad.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uimanager.h"

int KeyboardKey::sNumFlashes = 1;
float KeyboardKey::sSelectedMs = 40.0f;
float KeyboardKey::sNormalMs = 32.0f;

KeyboardKey::KeyboardKey(DataArray *pData, const char *pszPanel) : UIButton(pData, pszPanel) {
}

KeyboardKey::~KeyboardKey() {
}

bool KeyboardKey::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mButton != kPadCross || !mFlashDone || pMsg->mPressed == 0) {
        return false;
    }
    UIScreen *pScreen = TheUI.mCurrentScreen;
    UIComponentSelectStartMsg msg(this, TheUI.FocusPanel(), pScreen, pMsg->mButton, pMsg->mPad);
    if (!Dispatch(&msg)) {
        StartFlash(TheUI.mTime, sNumFlashes, kStateSelected, kStateNormal, sSelectedMs, sNormalMs);
    }
    return true;
}

void KeyboardKey::Flash() {
    StartFlash(TheUI.mTime, sNumFlashes, kStateSelected, kStateNormal, sSelectedMs, sNormalMs);
}

bool KeyboardKey::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIButton::DispatchPriv(pMsg);
}
