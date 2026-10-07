#include "met/gameoptionsscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/gameoptions.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kPanel[] = "o_set_game";
constexpr char kDoneComponent[] = "done";
constexpr char kFreqSizeComponent[] = "freqsize";
constexpr char kAudioComponent[] = "audio";
constexpr char kHelpTextComponent[] = "helptext";
constexpr char kVibrationComponent[] = "vibration";
constexpr char kSaveSettingsScreen[] = "save_settings";
constexpr char kToggleTokenFormat[] = "o_set_game_%s";
constexpr char kNoLabel[] = "NONE";

constexpr char kFreqLargeToken[] = "freq_large";
constexpr char kFreqSmallToken[] = "freq_small";
constexpr char kFreqHiddenToken[] = "freq_hidden";
constexpr char kOptionOnToken[] = "option_on";
constexpr char kOptionOffToken[] = "option_off";
constexpr char kAudioMonoToken[] = "audio_mono";
constexpr char kAudioStereoToken[] = "audio_stereo";
constexpr char kAudioSurroundToken[] = "audio_surround";

constexpr int kOutputMono = 0;
constexpr int kOutputStereo = 1;
constexpr int kOutputSurround = 2;
constexpr int kNumChoices = 3;

constexpr unsigned char kNoToggle = 0;

// Step a choice of three back or forward with the directional buttons, wrapping at either end.
inline int StepChoice(int nValue, int nButton) {
    if (nButton == kPadDLeft) {
        const int nPrev = nValue - 1;
        return nPrev > -1 ? nPrev : kNumChoices - 1;
    }
    if (nButton == kPadDRight || nButton == kPadCross) {
        const int nNext = nValue + 1;
        return nNext < kNumChoices ? nNext : 0;
    }
    return nValue;
}

} // namespace

bool GameOptionsScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void GameOptionsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mReturnScreen = pPrevScreen;
    Refresh();
}

void GameOptionsScreen::Refresh() {
    const GameOptions *pOptions = TheGameDb->GetOptions();
    SetToggle(kToggleHelpText, pOptions->mHelpText);
    SetToggle(kToggleForceFeedback, pOptions->mForceFeedback);
    mOutputMode = pOptions->mOutputMode;
    mFreqSize = pOptions->mFreqSize;
    RefreshToggle(kHelpTextComponent);
    RefreshToggle(kVibrationComponent);
    TheUI.FindComponent(kPanel, kFreqSizeComponent, false)->SetText(FreqSizeLabel(mFreqSize));
    TheUI.FindComponent(kPanel, kAudioComponent, false)->SetText(OutputModeLabel(mOutputMode));
}

void GameOptionsScreen::RefreshToggle(const char *pszComponent) {
    const unsigned char nToggle =
        strcmp(pszComponent, kHelpTextComponent) == 0 ? kToggleHelpText : kToggleForceFeedback;
    const char *pszLabel = OnOffLabel(IsToggleOn(nToggle));
    UIComponent *pComponent = TheUI.FindComponent(kPanel, pszComponent, false);
    const char *pszFormat =
        TheLocale.Localize(FormatString(kToggleTokenFormat, pszComponent), true);
    pComponent->SetText(FormatString(pszFormat, pszLabel));
}

void GameOptionsScreen::Commit() {
    GameOptions options;
    options.SetOutputMode(mOutputMode);
    options.SetHelpText(IsToggleOn(kToggleHelpText));
    options.SetForceFeedback(IsToggleOn(kToggleForceFeedback));
    options.SetFreqSize(mFreqSize);
    TheGameDb->SetOptions(&options);
}

bool GameOptionsScreen::IsToggleOn(unsigned char nToggle) const {
    return (mToggles & nToggle) != 0;
}

void GameOptionsScreen::FlipToggle(unsigned char nToggle) {
    if ((mToggles & nToggle) != 0) {
        mToggles &= ~nToggle;
    } else {
        mToggles |= nToggle;
    }
}

void GameOptionsScreen::SetToggle(unsigned char nToggle, int nOn) {
    if (nOn) {
        mToggles |= nToggle;
    } else {
        mToggles &= ~nToggle;
    }
}

const char *GameOptionsScreen::FreqSizeLabel(int nFreqSize) const {
    switch (nFreqSize) {
    case GameOptions::kFreqSizeLarge:
        return TheLocale.Localize(kFreqLargeToken, true);
    case GameOptions::kFreqSizeSmall:
        return TheLocale.Localize(kFreqSmallToken, true);
    case GameOptions::kFreqSizeHidden:
        return TheLocale.Localize(kFreqHiddenToken, true);
    default:
        return kNoLabel;
    }
}

const char *GameOptionsScreen::OnOffLabel(bool bOn) const {
    if (!bOn) {
        return TheLocale.Localize(kOptionOffToken, true);
    }
    return TheLocale.Localize(kOptionOnToken, true);
}

const char *GameOptionsScreen::OutputModeLabel(int nOutputMode) const {
    switch (nOutputMode) {
    case kOutputMono:
        return TheLocale.Localize(kAudioMonoToken, true);
    case kOutputStereo:
        return TheLocale.Localize(kAudioStereoToken, true);
    case kOutputSurround:
        return TheLocale.Localize(kAudioSurroundToken, true);
    default:
        return kNoLabel;
    }
}

bool GameOptionsScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    UIComponent *pComponent = pMsg->mComponent;
    const char *pszName = pComponent->mName;
    if (pMsg->mButton == kPadCross && strcmp(pszName, kDoneComponent) == 0) {
        Commit();
        if (TheMetagame.GetState() == Metagame::kStateFrontEnd) {
            TheUI.GotoScreen(kSaveSettingsScreen);
        } else if (mReturnScreen != nullptr) {
            TheUI.GotoScreen(mReturnScreen);
        }
    } else if (pMsg->mButton == kPadCross) {
        UIPanel *pPanel = TheUI.FindPanel(kPanel, false);
        pPanel->SetFocus(pPanel->FindComponent(kDoneComponent, false), kPadNone);
    } else if (strcmp(pszName, kFreqSizeComponent) == 0) {
        mFreqSize = StepChoice(mFreqSize, pMsg->mButton);
        pComponent->SetText(FreqSizeLabel(mFreqSize));
    } else if (strcmp(pszName, kAudioComponent) == 0) {
        mOutputMode = StepChoice(mOutputMode, pMsg->mButton);
        pComponent->SetText(OutputModeLabel(mOutputMode));
    } else {
        unsigned char nToggle = kToggleHelpText;
        if (strcmp(pszName, kHelpTextComponent) != 0) {
            nToggle = strcmp(pszName, kVibrationComponent) == 0 ?
                          static_cast<unsigned char>(kToggleForceFeedback) :
                          kNoToggle;
        }
        FlipToggle(nToggle);
        RefreshToggle(pszName);
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool GameOptionsScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadTriangle && mReturnScreen != nullptr) {
            TheUI.GotoScreen(mReturnScreen);
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}
