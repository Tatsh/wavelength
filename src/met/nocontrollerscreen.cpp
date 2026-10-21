#include "met/nocontrollerscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "met/dialogpanel.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

#ifdef VIDEO_STANDARD_PAL
namespace {

constexpr char kNoControllerDialog[] = "no_controller_dlg";

} // namespace
#endif

NoControllerScreen::NoControllerScreen(DataArray *pData) : FreqScreen(pData) {
    pData->FindBool("in_game", &mInGame, false);
}

void NoControllerScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    String port(TheMCManager.GetSlotName(mPad));
    DialogPanel *pDialog =
        mFocusPanel != nullptr ? dynamic_cast<DialogPanel *>(mFocusPanel) : nullptr;
#ifdef VIDEO_STANDARD_PAL
    // The multitap dialog has its own text.
    if (strcmp(pDialog->GetName(), kNoControllerDialog) == 0) {
        pDialog->SetText(FormatString(TheLocale.Localize(kNoControllerDialog, true), port.c_str()));
    }
#else
    pDialog->SetText(FormatString(TheLocale.Localize("no_controller_dlg", true), port.c_str()));
#endif
}

#ifdef VIDEO_STANDARD_PAL
bool NoControllerScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NoControllerScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->GetButton() == kPadCross) {
        const String button(pMsg->GetComponent()->GetName());
        if (button == "resume") {
            if (mInGame != 0) {
                TheMetagame.ShowEndGameScreens(Metagame::kDialogActionResume);
            } else {
                TheUI.GotoScreen(mReturnScreen.c_str());
            }
        } else {
            TheMetagame.ShowEndGameScreens(Metagame::kDialogActionEnd);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}
#else
bool NoControllerScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetPressed() != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->GetButton() == kPadCross && pMsg->GetPad() == mPad) {
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
#endif
