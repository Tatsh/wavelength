#include "met/freqmakercustomscreen.h"

#include <string.h>
#include <vector>

#include "game/gamedb.h"
#include "met/avatarpanel.h"
#include "met/freqmakererrorscreen.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

// A part button stays enabled while the player has unlocked a choice besides no choice at all.
constexpr unsigned kMinChoices = 2;

AvatarPanel *FindAvatarPanel() {
    return dynamic_cast<AvatarPanel *>(TheUI.FindPanel("f_maker_p", false));
}

// Disable a button of a panel when a list of choices offers no real choice.
void DisableWithoutChoice(UIPanel *pPanel,
                          const char *pszButton,
                          const std::vector<const char *> &choices) {
    if (choices.size() < kMinChoices) {
        pPanel->FindComponent(pszButton, false)->SetState(UIComponent::kStateDisabled, false);
    }
}

} // namespace

FreqMakerCustomScreen::FreqMakerCustomScreen(DataArray *pData)
    : FreqMakerUndoScreen(pData), mSavedValid(0), mChanged(0) {
}

FreqMakerCustomScreen::~FreqMakerCustomScreen() {
}

void FreqMakerCustomScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    if (mSavedValid == 0) {
        mSaved = *TheGameDb->GetAvatar(0);
        mSavedValid = 1;
    }
    if (pPrevScreen == TheUI.FindScreen("f_maker_lose_custom_changes", false)) {
        FindAvatarPanel()->SetAvatar(TheGameDb->GetAvatar(0));
    }
    mDoneButton = dynamic_cast<UIButton *>(TheUI.FindComponent("f_maker_c", "done", false));
    UpdateDone();
    if (pPrevScreen == TheUI.FindScreen("f_maker", false)) {
        mChanged = 0;
    }
    FreqScreen::Enter(pPrevScreen, fTime);

    std::vector<const char *> choices;
    Campaign *pProfile = TheGameDb->GetProfile(0);
    pProfile->GetUnlockedParts(AvatarPartSet::kPartHeadGear, &choices);
    DisableWithoutChoice(mFocusPanel, "head_gear", choices);
    pProfile->GetUnlockedParts(AvatarPartSet::kPartFaceGear, &choices);
    DisableWithoutChoice(mFocusPanel, "face_gear", choices);
    pProfile->GetUnlockedEmblems(&choices);
    DisableWithoutChoice(mFocusPanel, "emblems", choices);
}

const char *FreqMakerCustomScreen::Title() {
    return TheLocale.Localize("f_maker_c_TITLE", true);
}

void FreqMakerCustomScreen::UpdateDone() {
    (void)TheUI.FindPanel("f_maker_c", false); // Yes, the binary discards this lookup.
    if (TheGameDb->GetAvatar(0)->IsIncomplete()) {
        mDoneButton->SetState(UIComponent::kStateDisabled, false);
    } else {
        mDoneButton->SetState(UIComponent::kStateNormal, false);
    }
}

void FreqMakerCustomScreen::Undo() {
    AvatarPanel *pAvatarPanel = FindAvatarPanel();
    pAvatarPanel->SetAvatar(nullptr);
    *TheGameDb->GetAvatar(0) = mSaved;
    mSavedValid = 0;
    pAvatarPanel->SetAvatar(TheGameDb->GetAvatar(0));
}

bool FreqMakerCustomScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle && pMsg->mPressed != 0) {
        if (mChanged != 0) {
            FreqMakerErrorScreen *pError = dynamic_cast<FreqMakerErrorScreen *>(
                TheUI.FindScreen("f_maker_lose_custom_changes", false));
            pError->SetScreens("f_maker", "f_maker_custom");
            TheUI.GotoScreen(pError);
        } else {
            mSavedValid = 0;
            TheUI.GotoScreen(TheUI.FindScreen("f_maker", false));
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool FreqMakerCustomScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszButton = pMsg->mComponent->mName;
        if (strcmp(pszButton, "done") == 0) {
            mChanged = 0;
            mSavedValid = 0;
        } else {
            if (strcmp(pszButton, "clear") == 0) {
                TheGameDb->GetAvatar(0)->Clear();
                UpdateDone();
            }
            mChanged = 1;
        }
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool FreqMakerCustomScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
