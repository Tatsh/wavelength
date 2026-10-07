#include "met/remixcontrollerscreen.h"

#include "game/gamedb.h"
#include "os/joypad.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

RemixControllerScreen::RemixControllerScreen(DataArray *pData) : FreqScreen(pData) {
}

bool RemixControllerScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RemixControllerScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadCross && pMsg->mPressed != 0) {
        FxMidi::PlayMenuSelect();
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            TheUI.GotoScreen("pre_solotut2launchseq");
        } else {
            TheUI.GotoScreen("pre_multitut2launchseq");
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}
