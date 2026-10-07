#include "met/tipsscreen.h"

#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

// The value of mNumPages and mPage when the description does not specify them.
constexpr int kUnspecified = -1;

} // namespace

TipsScreen::TipsScreen(DataArray *pData)
    : FreqScreen(pData), mNextScreenName(nullptr), mPrevScreenName(nullptr) {
    mPage = kUnspecified;
    mNumPages = kUnspecified;
    pData->FindInt("page", &mPage, false);
    pData->FindInt("num_pages", &mNumPages, false);
    pData->FindSymbol("next_screen", &mNextScreenName, false);
    pData->FindSymbol("prev_screen", &mPrevScreenName, false);
}

void TipsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UILabel *pLabel =
        dynamic_cast<UILabel *>(TheUI.FindPanel("tips_help", false)->FindComponent("page", false));
    pLabel->SetText(FormatString(TheLocale.Localize("tips_page", true), mPage, mNumPages));
    FreqScreen::Enter(pPrevScreen, fTime);
}

bool TipsScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    const bool bHandled = FreqScreen::HandleJoypad(pMsg);
    if (!bHandled && pMsg->mPressed) {
        if (pMsg->mButton == kPadCross) {
            TheUI.GotoScreen(mNextScreenName);
        } else if (pMsg->mButton == kPadTriangle) {
            TheUI.GotoScreen(mPrevScreenName);
        }
    }
    return bHandled;
}

bool TipsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
