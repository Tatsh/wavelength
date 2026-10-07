#include "met/savefreqscreen.h"

#include <string.h>

#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "os/cheatsmanager.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

// How a save of the Freq ends, the values OnFreqSaved() receives.
constexpr int kSaveDone = 0;
constexpr int kSaveNoCard = 1;
constexpr int kSaveNoSpace = 2;
constexpr int kSaveUnformatted = 3;
constexpr int kSaveDifferentCard = 4;
constexpr int kSaveFreqExists = 5;
constexpr int kSaveFreqLimit = 6;

// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
constexpr int kOperationSave = 1;

ErrorScreen *FindErrorScreen(const char *pszName) {
    return dynamic_cast<ErrorScreen *>(TheUI.FindScreen(pszName, false));
}

} // namespace

SaveFreqScreen::SaveFreqScreen(DataArray *pData) : OverwriteSaveScreen(pData) {
    mIsCancel = 1;
    mOverwriteStatus = 0;
    pData->FindBool("overwrite_status", &mOverwriteStatus, false);
    pData->FindBool("isCancel", &mIsCancel, false);
}

SaveFreqScreen::~SaveFreqScreen() {
}

void SaveFreqScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    if (CheatsManager::IsCheatEntered() == 0) {
        return;
    }
    ErrorScreen *pScreen = FindErrorScreen("cheat_no_save_screen");
    pScreen->ClearTransitions();
    pScreen->AddTransition("ok", kPadNone, mDoneScreen.c_str());
    TheUI.GotoScreen(pScreen);
}

void SaveFreqScreen::OnFreqSaved(int nStatus, int nSpace) {
    ErrorScreen *pScreen;
    switch (nStatus) {
    case kSaveDone:
        TheUI.GotoScreen(mDoneScreen.c_str());
        TheGameDb->GetProfile(0)->SetModified(0);
        return;
    case kSaveNoCard:
    case kSaveUnformatted:
    case kSaveDifferentCard:
        if (mIsCancel != 0) {
            ShowCardError(nStatus);
        } else {
            ShowCardErrorTwoOption(nStatus, kOperationSave);
        }
        return;
    case kSaveNoSpace:
        ShowNoSpaceError(nSpace);
        return;
    case kSaveFreqLimit:
        pScreen = FindErrorScreen("freq_limit_error");
        pScreen->SetSlot(mSlot);
        pScreen->ClearTransitions();
        pScreen->AddTransition("retry", kPadNone, mName);
        pScreen->AddTransition("continue", kPadNone, mDoneScreen.c_str());
        break;
    case kSaveFreqExists:
        pScreen = FindErrorScreen("freq_exists_error");
        pScreen->SetSlot(mSlot);
        pScreen->ClearTransitions();
        pScreen->AddTransition("retry", kPadNone, mName);
        pScreen->AddTransition("cancel", kPadNone, mStartScreen.c_str());
        pScreen->SetStartScreen(mName);
        break;
    default:
        DebugWarn("not handled save freq! GET CHRISTINE\n");
        return;
    }
    TheUI.GotoScreen(pScreen);
}

bool SaveFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.SaveFreq(
            this, mSlot, TheGameDb->GetProfile(0), TheGameDb->GetPlayerName(0), mOverwriteStatus);
    }
    return false;
}

bool SaveFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
