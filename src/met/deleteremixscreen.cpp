#include "met/deleteremixscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

#ifdef VIDEO_STANDARD_PAL
// The dialog of a card without the remix.
constexpr char kNotFoundScreen[] = "load_remix_not_found_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";
#else
// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
constexpr int kOperationDelete = 3;
#endif

} // namespace

#ifdef VIDEO_STANDARD_PAL
void DeleteRemixScreen::OnRemixDeleted(int nStatus) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheUI.GotoScreen(mDoneScreen.c_str());
        return;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusFull:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kOperationDelete);
        return;
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusNotFound: {
        UIScreen *pError = TheUI.FindScreen(kNotFoundScreen, false);
        pError->ClearTransitions();
        pError->AddTransition(kRetryComponent, kPadNone, mName);
        pError->AddTransition(kCancelComponent, kPadNone, mStartScreen.c_str());
        TheUI.GotoScreen(pError);
        return;
    }
    default:
        DebugWarn("not handled delete remix! GET CHRISTINE\n");
        return;
    }
}
#else
void DeleteRemixScreen::OnRemixDeleted(int nStatus) {
    if (nStatus == MemcardTask::kStatusOk) {
        TheUI.GotoScreen(mDoneScreen.c_str());
    } else if (nStatus > MemcardTask::kStatusOk && nStatus < MemcardTask::kStatusExists) {
        ShowCardErrorTwoOption(nStatus, kOperationDelete);
    } else {
        DebugWarn("not handled delete remix! GET CHRISTINE\n");
    }
}
#endif

bool DeleteRemixScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool DeleteRemixScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->GetScreen()->GetName(), mName) == 0) {
        TheMCManager.DeleteRemix(this, mSlot, mRemixName.c_str());
    }
    return false;
}
