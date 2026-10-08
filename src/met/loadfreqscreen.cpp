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

} // namespace

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
