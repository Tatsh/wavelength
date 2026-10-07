#include "met/pausescreen.h"

#include <string.h>

#include "met/metagame.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"

namespace {

constexpr char kResumeButton[] = "resume";

} // namespace

PauseScreen::PauseScreen(DataArray *pData) : FreqScreen(pData), mPad(0) {
}

void PauseScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mFocusPanel->SetFocus(mFocusPanel->FindComponent(kResumeButton, false), kPadNone);
    FreqScreen::Enter(pPrevScreen, fTime);
}

bool PauseScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszName = pMsg->mComponent->mName;
        if (strcmp(pszName, kResumeButton) == 0) {
            TheMetagame.ShowEndGameScreens(Metagame::kDialogActionResume);
        } else if (strcmp(pszName, "quit") == 0) {
            TheMetagame.ShowEndGameScreens(Metagame::kDialogActionEnd);
        } else if (strcmp(pszName, "restart") == 0) {
            TheMetagame.ShowEndGameScreens(Metagame::kDialogActionQuit);
        } else if (strcmp(pszName, "controller_config") != 0) {
            (void)strcmp(pszName, "game_settings"); // Yes, the binary discards this comparison.
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool PauseScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mPad != mPad) {
        return true;
    }
    if (pMsg->mPressed && pMsg->mButton == kPadStart) {
        TheMetagame.ShowEndGameScreens(Metagame::kDialogActionResume);
    }
    return false;
}

bool PauseScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
