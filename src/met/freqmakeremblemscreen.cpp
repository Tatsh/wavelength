#include "met/freqmakeremblemscreen.h"

#include <string.h>

#include "game/avatarcam.h"
#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

FreqMakerEmblemScreen::FreqMakerEmblemScreen(DataArray *pData)
    : FreqScreen(pData), mOriginalIndex(0), mIndex(0), mPartButton(nullptr) {
}

FreqMakerEmblemScreen::~FreqMakerEmblemScreen() {
}

void FreqMakerEmblemScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    TheGameDb->GetProfile(0)->GetUnlockedEmblems(&mEmblems);
    const char *pszCurrent = TheGameDb->GetAvatar(0)->EmblemName();
    unsigned i = 0;
    for (; i < mEmblems.size(); ++i) {
        if (strcmp(mEmblems[i], pszCurrent) == 0) {
            mIndex = static_cast<int>(i);
            mOriginalIndex = static_cast<int>(i);
            break;
        }
    }
    if (i == mEmblems.size()) {
        mEmblems.push_back(pszCurrent);
        mOriginalIndex = static_cast<int>(mEmblems.size()) - 1;
        mIndex = static_cast<int>(mEmblems.size()) - 1;
    }

    SetAvatarCam("emblems");
    UIPanel *pPanel = TheUI.FindPanel("f_maker_e", false);
    mPartButton = dynamic_cast<UIButton *>(pPanel->FindComponent("part", false));
    mPartButton->SetText(TheLocale.Localize(mEmblems[mIndex], true));
    pPanel->SetFocus(mPartButton, kPadNone);
    mReserved78 = 1;
}

void FreqMakerEmblemScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    SetAvatarCam("f_maker");
}

const char *FreqMakerEmblemScreen::Title() {
    return TheLocale.Localize("f_maker_c_TITLE", true);
}

bool FreqMakerEmblemScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mButton == kPadCross && strcmp(pMsg->mComponent->mName, "done") != 0) {
        UIPanel *pPanel = TheUI.FindPanel("f_maker_e", false);
        pPanel->SetFocus(pPanel->FindComponent("done", false), kPadNone);
    }
    if (strcmp(pMsg->mComponent->mName, "part") == 0) {
        const int nEmblems = static_cast<int>(mEmblems.size());
        if (pMsg->mButton == kPadDLeft) {
            mIndex = mIndex - 1 > -1 ? mIndex - 1 : nEmblems - 1;
        } else if (pMsg->mButton == kPadDRight) {
            mIndex = mIndex + 1 < nEmblems ? mIndex + 1 : 0;
        }
        const char *pszEmblem = mEmblems[mIndex];
        mPartButton->SetText(TheLocale.Localize(pszEmblem, true));
        TheGameDb->GetAvatar(0)->SetEmblem(pszEmblem);
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool FreqMakerEmblemScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle && pMsg->mPressed != 0) {
        const char *pszEmblem = mEmblems[mOriginalIndex];
        TheGameDb->GetAvatar(0)->SetEmblem(pszEmblem);
        TheUI.GotoScreen(TheUI.FindScreen("f_maker_custom", false));
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool FreqMakerEmblemScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
