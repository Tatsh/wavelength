#include "met/copyfreqscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "ui/uimanager.h"

namespace {

// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
#ifdef VIDEO_STANDARD_PAL
constexpr int kOperationCopy = ErrorScreen::kOperationCopy;
#else
constexpr int kOperationCopy = 2;
#endif

} // namespace

void CopyFreqScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
}

void CopyFreqScreen::OnFreqSaved(int nStatus, int nSpace) {
    switch (nStatus) {
    case MemcardTask::kStatusOk:
        TheUI.GotoScreen(mDoneScreen.c_str());
        break;
    case MemcardTask::kStatusNoCard:
    case MemcardTask::kStatusUnformatted:
    case MemcardTask::kStatusChangedCard:
        ShowCardErrorTwoOption(nStatus, kOperationCopy);
        break;
    default:
        SaveFreqScreen::OnFreqSaved(nStatus, nSpace);
        break;
    }
}

bool CopyFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return SaveFreqScreen::DispatchPriv(pMsg);
}

bool CopyFreqScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->GetName(), mName) == 0) {
        TheMCManager.SaveFreq(this, mSlot, &mProfile, mProfile.mName.c_str(), mOverwriteStatus);
    }
    return false;
}
