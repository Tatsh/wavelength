#include "met/mccheckscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/metagame.h"
#include "met/transitionerrorscreen.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uimanager.h"

namespace {

constexpr char kNoCardScreen[] = "mc_check_no_card";
constexpr char kNoSpaceScreen[] = "mc_check_no_space";
constexpr char kUnformattedScreen[] = "mc_check_unformatted";
constexpr char kFormatScreen[] = "mc_format";
constexpr char kWarningScreen[] = "startup_warning";
constexpr char kNoCardToken[] = "mc_check_no_card_dlg";
constexpr char kNoSpaceToken[] = "mc_check_no_space_dlg";
constexpr char kRetryComponent[] = "retry";
constexpr char kContinueComponent[] = "continue";
constexpr char kYesComponent[] = "yes";
constexpr char kNoComponent[] = "no";

// The first player.
constexpr int kFirstPlayer = 0;

// The milliseconds between the load of the Freq and the move to the done screen.
constexpr float kAdvanceDelayMs = 1000.0f;

ErrorScreen *FindErrorScreen(const char *pszName) {
    return dynamic_cast<ErrorScreen *>(TheUI.FindScreen(pszName, false));
}

} // namespace

void MCCheckScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    if (TheGameDb->GetProfile(kFirstPlayer)->mCustom != 0) {
        return;
    }
    TheGameDb->ClearPlayers();
    Campaign profile;
    TheGameDb->AddPlayer(&profile);
}

void MCCheckScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mLoadSettings != 0) {
        mLoadSettings = 0;
        TheMCManager.LoadSettings(this, mSlot);
    } else if (mLoadFreqs != 0) {
        mLoadFreqs = 0;
        TheMCManager.LoadFreqs(this, mSlot);
    }
    if (mAdvanceTime != 0.0f && mAdvanceTime < fTime) {
        mAdvanceTime = 0.0f;
        TheUI.GotoScreen(mDoneScreen.c_str());
    }
}

bool MCCheckScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool MCCheckScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        mLoadFreqs = 0;
        mLoadSettings = 0;
        mAdvanceTime = 0.0f;
        TheMCManager.InitialCheck(this, mSlot);
    }
    return false;
}

void MCCheckScreen::OnInitialCheck(int nStatus, int nFormat, int nFree, int nNeeded) {
    UIScreen *pError = nullptr;
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        if (nFormat == 0) {
            pError = TheUI.FindScreen(kUnformattedScreen, false);
            break;
        }
        TheMetagame.mFreqsOnCard = 1;
        if (!TheUI.mEditMode) {
            mLoadSettings = 1;
        } else {
            TheUI.GotoScreen(mDoneScreen.c_str());
        }
        return;
    case MemcardTask::kStatusNoCard: {
        pError = TheUI.FindScreen(kNoCardScreen, false);
        auto *pDialog = dynamic_cast<TransitionErrorScreen *>(pError);
        const String format(TheLocale.Localize(kNoCardToken, true));
        const String text(FormatString(format.c_str(), GetSlotName(), GetSlotName()));
        pDialog->mMessage = text.c_str();
        break;
    }
    case MemcardTask::kStatusFull: {
        pError = TheUI.FindScreen(kNoSpaceScreen, false);
        auto *pDialog = dynamic_cast<TransitionErrorScreen *>(pError);
        const String format(TheLocale.Localize(kNoSpaceToken, true));
        const String text(FormatString(format.c_str(), GetSlotName(), nFree, nNeeded));
        pDialog->mMessage = text.c_str();
        break;
    }
    case MemcardTask::kStatusUnformatted: {
        ErrorScreen *pFormat = FindErrorScreen(kFormatScreen);
        pFormat->SetDoneScreen(mName);
        pFormat->SetStartScreen(mName);
        ErrorScreen *pUnformatted = FindErrorScreen(kUnformattedScreen);
        pUnformatted->SetSlot(mSlot);
        pUnformatted->ClearTransitions();
        pUnformatted->AddTransition(kYesComponent, kPadNone, kFormatScreen);
        pUnformatted->AddTransition(kNoComponent, kPadNone, kWarningScreen);
        TheUI.GotoScreen(pUnformatted);
        return;
    }
    default:
        TheMetagame.mFreqsOnCard = 0;
        TheUI.GotoScreen(mDoneScreen.c_str());
        return;
    }

    if (pError == nullptr) {
        return;
    }
    TheMetagame.mFreqsOnCard = 0;
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, mName);
    pError->AddTransition(kContinueComponent, kPadNone, mDoneScreen.c_str());
    TheUI.GotoScreen(pError);
}

void MCCheckScreen::OnSettingsLoaded(int nStatus, GameOptions settings) {
    if (nStatus == MemcardTask::kStatusOk) {
        TheGameDb->SetOptions(&settings);
    }
    mLoadFreqs = 1;
}

void MCCheckScreen::OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) {
    if (nStatus != MemcardTask::kStatusOk) {
        TheMetagame.mFreqsOnCard = 0;
        TheUI.GotoScreen(mDoneScreen.c_str());
        return;
    }
    if (!pProfiles->empty()) {
        TheGameDb->ClearPlayers();
        TheGameDb->AddPlayer(&pProfiles->front());
    } else {
        TheMetagame.mFreqsOnCard = 0;
    }
    mAdvanceTime = TheUI.mTime + kAdvanceDelayMs;
}
