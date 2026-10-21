#include "met/saveeditedfreqscreen.h"

#include <string.h>

#include "game/gamedb.h"
#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

SaveEditedFreqScreen::SaveEditedFreqScreen(DataArray *pData)
    : SaveFreqScreen(pData), mFreqName("") {
}

SaveEditedFreqScreen::~SaveEditedFreqScreen() {
}

bool SaveEditedFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->GetScreen()->GetName(), mName) == 0) {
        TheMCManager.SaveFreq(
            this, mSlot, TheGameDb->GetProfile(0), mFreqName.c_str(), mOverwriteStatus);
    }
    return false;
}

#ifdef VIDEO_STANDARD_PAL
void SaveEditedFreqScreen::OnFreqSaved(int nStatus, int nSpace) {
    if (nStatus != MemcardTask::kStatusChangedCard || mOverwriteStatus == 0) {
        SaveFreqScreen::OnFreqSaved(nStatus, nSpace);
        return;
    }
    mOverwriteStatus = 0;
    auto *pError = dynamic_cast<ErrorScreen *>(TheUI.FindScreen("3opt_diff_card_error", false));
    pError->ClearTransitions();
    pError->SetSlot(mSlot, kOperationSave);
    pError->AddTransition("save", kPadNone, mName);
    pError->AddTransition("cancel", kPadNone, mStartScreen.c_str());
    pError->AddTransition("continue", kPadNone, mDoneScreen.c_str());
    TheUI.GotoScreen(pError);
}
#endif

bool SaveEditedFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return SaveFreqScreen::DispatchPriv(pMsg);
}
