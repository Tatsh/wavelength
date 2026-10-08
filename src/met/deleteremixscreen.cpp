#include "met/deleteremixscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "os/debug.h"
#include "ui/uimanager.h"

namespace {

// The operation ErrorScreen::ShowCardErrorTwoOption() reports a failure of.
constexpr int kOperationDelete = 3;

} // namespace

void DeleteRemixScreen::OnRemixDeleted(int nStatus) {
    if (nStatus == MemcardTask::kStatusOk) {
        TheUI.GotoScreen(mDoneScreen.c_str());
    } else if (nStatus > MemcardTask::kStatusOk && nStatus < MemcardTask::kStatusExists) {
        ShowCardErrorTwoOption(nStatus, kOperationDelete);
    } else {
        DebugWarn("not handled delete remix! GET CHRISTINE\n");
    }
}

bool DeleteRemixScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool DeleteRemixScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.DeleteRemix(this, mSlot, mRemixName.c_str());
    }
    return false;
}
