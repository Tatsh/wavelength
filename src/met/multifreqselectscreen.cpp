#include "met/multifreqselectscreen.h"

#include <cstring>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "met/freqselpanel.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanelFormat[] = "m_g_s_f_%dpl_0%d";
constexpr char kDuelBackScreen[] = "m_mode";
constexpr char kBackScreen[] = "m_player";
constexpr char kDefaultNameFormat[] = "default_name_%d";
constexpr char kNoName[] = "";
constexpr char kDuelDoneScreen[] = "multifreq2multiskill_duel";
constexpr char kRemixDoneScreen[] = "multifreq2multimode_remix";
constexpr char kDoneScreen[] = "multifreq2multiskill";

FreqSelPanel *FindFreqSelPanel(int nPlayer) {
    const char *pszPanel = FormatString(kPanelFormat, TheGameDb->GetNumPlayers(), nPlayer + 1);
    return dynamic_cast<FreqSelPanel *>(TheUI.FindPanel(pszPanel, false));
}

} // namespace

MultiFreqSelectScreen::MultiFreqSelectScreen(DataArray *pData) : FreqScreen(pData) {
    pData->FindInt("num_players", &mNumPlayers, false);
}

void MultiFreqSelectScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mConfirmed.clear();
    mConfirmed.resize(mNumPlayers);
    // Yes, the binary clears every bit again after the resize.
    for (int i = 0; i < mNumPlayers; ++i) {
        mConfirmed[i] = false;
    }
}

bool MultiFreqSelectScreen::DispatchPriv(Message *pMsg) {
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

bool MultiFreqSelectScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const int nPad = pMsg->mPad;
    if (nPad >= mNumPlayers) {
        return false;
    }
    const char *pszPanel = FormatString(kPanelFormat, TheGameDb->GetNumPlayers(), nPad + 1);
    auto *pPanel = dynamic_cast<FreqSelPanel *>(TheUI.FindPanel(pszPanel, false));
    return pPanel->HandleSelectStart(pMsg);
}

bool MultiFreqSelectScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    bool bHandled = false;
    if (pMsg->mButton != kPadCross) {
        return bHandled;
    }
    const char *pszName = pMsg->mComponent->mName;
    const int nDigit = pszName[std::strlen(pszName) - 1];
    const int nPlayer = nDigit - '1';
    if (nPlayer < mNumPlayers) {
        const char *pszPanel = FormatString(kPanelFormat, TheGameDb->GetNumPlayers(), nDigit - '0');
        auto *pPanel = dynamic_cast<FreqSelPanel *>(TheUI.FindPanel(pszPanel, false));
        bHandled = pPanel->HandleSelect(pMsg);
        mConfirmed[nPlayer] = true;
        CheckDone();
    }
    return bHandled;
}

bool MultiFreqSelectScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    const int nPad = pMsg->mPad;
    if (nPad >= mNumPlayers) {
        return FreqScreen::HandleJoypad(pMsg);
    }
    if (pMsg->mPressed != 0 && pMsg->mButton == kPadTriangle) {
        if (mConfirmed[nPad]) {
            mConfirmed[nPad] = false;
            FindFreqSelPanel(nPad)->SetChosen(false);
            FxMidi::PlayBack();
            return FreqScreen::HandleJoypad(pMsg);
        }
        bool bAnyConfirmed = false;
        for (int i = 0; i < mNumPlayers; ++i) {
            bAnyConfirmed |= mConfirmed[i];
        }
        if (!bAnyConfirmed) {
            TheUI.GotoScreen(TheGameDb->mRuleSet == GameDb::kRuleSetDuel ? kDuelBackScreen :
                                                                           kBackScreen);
        }
        return false;
    }
    if (!mConfirmed[nPad]) {
        FindFreqSelPanel(nPad)->Dispatch(pMsg); // Yes, the binary discards the result.
    }
    return FreqScreen::HandleJoypad(pMsg);
}

void MultiFreqSelectScreen::CheckDone() {
    bool bAllConfirmed = true;
    for (int i = 0; i < mNumPlayers; ++i) {
        bAllConfirmed &= mConfirmed[i];
    }
    if (!bAllConfirmed) {
        return;
    }
    for (int i = 0; i < mNumPlayers; ++i) {
        if (std::strcmp(TheGameDb->GetProfile(i)->mName.c_str(), kNoName) == 0) {
            TheGameDb->GetProfile(i)->mName =
                TheLocale.Localize(FormatString(kDefaultNameFormat, i + 1), true);
        }
    }
    const char *pszScreen;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        pszScreen = kDuelDoneScreen;
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        pszScreen = kRemixDoneScreen;
    } else {
        pszScreen = kDoneScreen;
    }
    TheUI.GotoScreen(pszScreen);
}
