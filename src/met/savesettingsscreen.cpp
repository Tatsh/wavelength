#include "met/savesettingsscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kExistsScreen[] = "settings_exist_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";

} // namespace

void SaveSettingsScreen::OnSettingsSaved(int nStatus, int nNeeded) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheGameDb->GetOptions()->mModified = 0;
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
        UIScreen *pScreen = TheUI.FindScreen(kExistsScreen, false);
        pScreen->ClearTransitions();
        pScreen->AddTransition(kRetryComponent, kPadNone, mName);
        pScreen->AddTransition(kCancelComponent, kPadNone, mDoneScreen.c_str());
        auto *pError = pScreen != nullptr ? dynamic_cast<ErrorScreen *>(pScreen) : nullptr;
        pError->SetStartScreen(mName);
        TheUI.GotoScreen(pScreen);
        break;
    }
    default:
        DebugWarn("not handled save settings! GET CHRISTINE\n");
        break;
    }
}

bool SaveSettingsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SaveSettingsScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.SaveSettings(this, mSlot, TheGameDb->GetOptions(), mOverwriteStatus);
    }
    return false;
}
