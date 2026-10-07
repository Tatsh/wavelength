#include "met/netparamsscreen.h"

#include <cstring>

#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kSongPanelEntry[] = "song_panel_name";
constexpr char kButtonPanelEntry[] = "button_panel_name";
constexpr char kTriBackEntry[] = "tri_back";
constexpr char kModeComponent[] = "mode";
constexpr char kSkillComponent[] = "skill";
constexpr char kSongComponent[] = "song";
constexpr char kCursorComponent[] = "cursor";
constexpr char kListComponent[] = "list";

constexpr int kNoChange = 0;

// Move a choice one step through a list, wrapping at both ends.
int StepChoice(int nChoice, int nCount, int nButton) {
    if (nButton == kPadDLeft) {
        return nChoice - 1 > -1 ? nChoice - 1 : nCount - 1;
    }
    return nChoice + 1 < nCount ? nChoice + 1 : 0;
}

} // namespace

NetParamsScreen::NetParamsScreen(DataArray *pData) : FreqScreen(pData) {
    mMode = 0;
    mSkill = 0;
    mLastSkillMode = 0;
    mReservedB4 = 0;
    mReserved7C = 1;
    pData->FindSymbol(kSongPanelEntry, &mSongPanelName, false);
    pData->FindSymbol(kButtonPanelEntry, &mButtonPanelName, false);
    pData->FindSymbol(kTriBackEntry, &mTriBackScreen, false);
}

void NetParamsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mSongPanel = TheUI.FindPanel(mSongPanelName, false);
    SetFocus(TheUI.FindPanel(mButtonPanelName, false));
    UpdateLabels();
}

void NetParamsScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    mModes.clear();
    mSkills.clear();
}

void NetParamsScreen::UpdateLabels() {
    UIPanel *pPanel = TheUI.FindPanel(mButtonPanelName, false);
    pPanel->FindComponent(kModeComponent, false)->SetText(mModes[mMode].c_str());
    UIComponent *pSkill = pPanel->FindComponent(kSkillComponent, false);
    if (mLastSkillMode < mMode) {
        pSkill->SetState(UIComponent::kStateDisabled, false);
    } else if (pPanel->mFocus == pSkill) {
        pSkill->SetState(UIComponent::kStateSelected, false);
    } else {
        pSkill->SetState(UIComponent::kStateNormal, false);
    }
    pPanel->FindComponent(kSkillComponent, false)->SetText(mSkills[mSkill].c_str());
}

bool NetParamsScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetParamsScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszComponent = pMsg->mComponent->mName;
        if (std::strcmp(pszComponent, kSongComponent) == 0) {
            SetFocus(TheUI.FindPanel(mSongPanelName, false));
        } else if (std::strcmp(pszComponent, kCursorComponent) == 0) {
            SetFocus(TheUI.FindPanel(mButtonPanelName, false));
            static_cast<UIList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false))
                ->SetCursorSelected(true);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetParamsScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszComponent = pMsg->mComponent->mName;
    const int nButton = pMsg->mButton;
    if (std::strcmp(pszComponent, kModeComponent) == 0) {
        if (nButton == kPadDLeft || nButton == kPadDRight) {
            mMode = StepChoice(mMode, static_cast<int>(mModes.size()), nButton);
        }
        OnModeChanged();
        OnChoiceChanged(kNoChange);
        UpdateLabels();
    } else if (std::strcmp(pszComponent, kSkillComponent) == 0) {
        if (nButton == kPadDLeft || nButton == kPadDRight) {
            mSkill = StepChoice(mSkill, static_cast<int>(mSkills.size()), nButton);
        }
        OnChoiceChanged(kNoChange);
        UpdateLabels();
    } else if (std::strcmp(pszComponent, kSongComponent) == 0 && nButton == kPadDRight) {
        SetFocus(TheUI.FindPanel(mSongPanelName, false));
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool NetParamsScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (mFocusPanel == TheUI.FindPanel(mSongPanelName, false) &&
        (pMsg->mButton == kPadTriangle || pMsg->mButton == kPadDLeft)) {
        SetFocus(TheUI.FindPanel(mButtonPanelName, false));
        static_cast<UIList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false))
            ->SetCursorSelected(true);
        return false;
    }
    if (pMsg->mPressed != 0 && pMsg->mButton == kPadTriangle) {
        TheUI.GotoScreen(mTriBackScreen);
    }
    return false;
}
