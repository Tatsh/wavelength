#include "met/saveremixscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kExistsScreen[] = "remix_exists_error";
constexpr char kLimitScreen[] = "remix_limit_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";
constexpr char kContinueComponent[] = "continue";

} // namespace

SaveRemixScreen::SaveRemixScreen(DataArray *pData) : OverwriteSaveScreen(pData) {
    mOverwriteStatus = 0; // The binary also clears it before it constructs mRemixName.
}

void SaveRemixScreen::OnRemixSaved(int nStatus, int nNeeded) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheUI.GotoScreen(mDoneScreen.c_str());
        break;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusChangedCard:
        ShowCardError(nStatus);
        break;
    case MemcardTask::kStatusFull:
        ShowNoSpaceError(nNeeded);
        break;
    case MemcardTask::kStatusExists: {
        auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(kExistsScreen, false));
        pError->ClearTransitions();
        pError->AddTransition(kRetryComponent, kPadNone, mName);
        pError->AddTransition(kCancelComponent, kPadNone, mStartScreen.c_str());
        pError->SetStartScreen(mName);
        TheUI.GotoScreen(pError);
        break;
    }
    case MemcardTask::kStatusLimit: {
        UIScreen *pError = TheUI.FindScreen(kLimitScreen, false);
        pError->ClearTransitions();
        pError->AddTransition(kRetryComponent, kPadNone, mName);
        pError->AddTransition(kContinueComponent, kPadNone, mDoneScreen.c_str());
        TheUI.GotoScreen(pError);
        break;
    }
    default:
        DebugWarn("not handled save remix! GET CHRISTINE\n");
        break;
    }
}

bool SaveRemixScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SaveRemixScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        RemixInfo info = *TheGameDb->GetRemixInfo();
        std::strcpy(info.mName, mRemixName.c_str());
        info.mSource = RemixInfo::kSourceMemcard;
        TheGameDb->SetRemix(&info);
        TheMCManager.SaveRemix(this, mSlot, &info, TheGameDb->GetRemixBuffer(), mOverwriteStatus);
    }
    return false;
}
