#include "met/nocontrollerscreen.h"

#include "memcard/mcmanager.h"
#include "met/dialogpanel.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

NoControllerScreen::NoControllerScreen(DataArray *pData) : FreqScreen(pData) {
    pData->FindBool("in_game", &mInGame, false);
}

void NoControllerScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    String port(TheMCManager.GetSlotName(mPad));
    DialogPanel *pDialog =
        mFocusPanel != nullptr ? dynamic_cast<DialogPanel *>(mFocusPanel) : nullptr;
    pDialog->SetText(FormatString(TheLocale.Localize("no_controller_dlg", true), port.c_str()));
}

bool NoControllerScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadCross && pMsg->mPad == mPad) {
            if (mInGame != 0) {
                TheMetagame.ShowEndGameScreens(Metagame::kDialogActionResume);
            } else {
                TheUI.GotoScreen(mReturnScreen.c_str());
            }
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool NoControllerScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
