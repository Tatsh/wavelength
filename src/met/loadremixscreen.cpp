#include "met/loadremixscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kNotFoundScreen[] = "load_remix_not_found_error";
constexpr char kFailedScreen[] = "load_failed_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";

// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;

// Go to an error screen whose `retry` returns to the loading screen and whose `cancel` returns to
// the start screen.
inline void ShowLoadError(const char *pszError, const char *pszRetry, const char *pszCancel) {
    UIScreen *pError = TheUI.FindScreen(pszError, false);
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, pszRetry);
    pError->AddTransition(kCancelComponent, kPadNone, pszCancel);
    TheUI.GotoScreen(pError);
}

} // namespace

void LoadRemixScreen::OnRemixLoaded(int nStatus) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheUI.GotoScreen(mDoneScreen.c_str());
        break;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kNoOperation);
        break;
    case MemcardTask::kStatusNotFound:
        ShowLoadError(kNotFoundScreen, mName, mStartScreen.c_str());
        break;
    default:
        ShowLoadError(kFailedScreen, mName, mStartScreen.c_str());
        break;
    }
}

bool LoadRemixScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool LoadRemixScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        RemixInfo info = *TheGameDb->GetRemixInfo();
        TheGameDb->SetRemixBuffer(info.mDataSize);
        TheMCManager.LoadRemix(
            this, mSlot, info.mName, TheGameDb->GetRemixBuffer(), info.mDataSize);
    }
    return false;
}
