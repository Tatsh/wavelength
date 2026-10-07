#include "met/saveeditedfreqscreen.h"

#include <string.h>

#include "game/gamedb.h"
#include "memcard/mcmanager.h"

SaveEditedFreqScreen::SaveEditedFreqScreen(DataArray *pData)
    : SaveFreqScreen(pData), mFreqName("") {
}

SaveEditedFreqScreen::~SaveEditedFreqScreen() {
}

bool SaveEditedFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.SaveFreq(
            this, mSlot, TheGameDb->GetProfile(0), mFreqName.c_str(), mOverwriteStatus);
    }
    return false;
}

bool SaveEditedFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return SaveFreqScreen::DispatchPriv(pMsg);
}
