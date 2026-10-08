#include "met/formatscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kAlreadyFormattedScreen[] = "already_formatted_error";
constexpr char kFailedScreen[] = "format_card_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";

// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
constexpr int kOperationFormat = 4;

} // namespace

void FormatScreen::OnCardFormatted(int nStatus) {
    const char *pszError;
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheUI.GotoScreen(mDoneScreen.c_str());
        return;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kOperationFormat);
        return;
    case MemcardTask::kStatusFormatted:
        pszError = kAlreadyFormattedScreen;
        break;
    default:
        pszError = kFailedScreen;
        break;
    }
    auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(pszError, false));
    pError->SetSlot(mSlot);
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, mName);
    pError->AddTransition(kCancelComponent, kPadNone, mDoneScreen.c_str());
    TheUI.GotoScreen(pError);
}

bool FormatScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool FormatScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.FormatCard(this, mSlot);
    }
    return false;
}
