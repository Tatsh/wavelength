#include "met/loadfreqscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/selloadedfreqscreen.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kNoFreqsScreen[] = "no_freqs_loaded_error";
constexpr char kFailedScreen[] = "load_failed_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";

#ifdef VIDEO_STANDARD_PAL
// Go to an error dialog about a slot whose `retry` returns to the loading screen and whose
// `cancel` returns to the start screen.
inline void ShowLoadError(
    const char *pszError, int nSlot, int nOperation, const char *pszRetry, const char *pszCancel) {
    auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(pszError, false));
    pError->SetSlot(nSlot, nOperation);
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, pszRetry);
    pError->AddTransition(kCancelComponent, kPadNone, pszCancel);
    TheUI.GotoScreen(pError);
}
#else
// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;

// Go to an error dialog about a slot whose `retry` returns to the loading screen and whose
// `cancel` returns to the start screen.
inline void
ShowLoadError(const char *pszError, int nSlot, const char *pszRetry, const char *pszCancel) {
    auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(pszError, false));
    pError->SetSlot(nSlot);
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, pszRetry);
    pError->AddTransition(kCancelComponent, kPadNone, pszCancel);
    TheUI.GotoScreen(pError);
}
#endif

} // namespace

#ifdef VIDEO_STANDARD_PAL
void LoadFreqScreen::OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        if (!pProfiles->empty()) {
            auto *pScreen =
                dynamic_cast<SelLoadedFreqScreen *>(TheUI.FindScreen(mDoneScreen.c_str(), false));
            pScreen->SetProfiles(*pProfiles);
            TheUI.GotoScreen(pScreen);
            return;
        }
        ShowLoadError(kNoFreqsScreen, mSlot, kOperationLoad, mName, mStartScreen.c_str());
        return;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusFull:
        ShowCardErrorTwoOption(nStatus, kOperationLoad);
        return;
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusNotFound:
        ShowLoadError(kNoFreqsScreen, mSlot, kOperationLoad, mName, mStartScreen.c_str());
        return;
    case MemcardTask::kStatusChangedCard:
        mReload = 1;
        return;
    default:
        ShowLoadError(kFailedScreen, mSlot, kOperationNone, mName, mStartScreen.c_str());
        return;
    }
}

void LoadFreqScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mReload != 0) {
        mReload = 0;
        TheMCManager.LoadFreqs(this, mSlot);
    }
}
#else
void LoadFreqScreen::OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) {
    if (nStatus == MemcardTask::kStatusOk && !pProfiles->empty()) {
        auto *pScreen =
            dynamic_cast<SelLoadedFreqScreen *>(TheUI.FindScreen(mDoneScreen.c_str(), false));
        pScreen->SetProfiles(*pProfiles);
        TheUI.GotoScreen(pScreen);
    } else if (nStatus == MemcardTask::kStatusOk || nStatus == MemcardTask::kStatusNotFound) {
        ShowLoadError(kNoFreqsScreen, mSlot, mName, mStartScreen.c_str());
    } else if (nStatus > MemcardTask::kStatusOk && nStatus < MemcardTask::kStatusExists) {
        ShowCardErrorTwoOption(nStatus, kNoOperation);
    } else {
        ShowLoadError(kFailedScreen, mSlot, mName, mStartScreen.c_str());
    }
}
#endif

bool LoadFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool LoadFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.LoadFreqs(this, mSlot);
    }
    return false;
}
