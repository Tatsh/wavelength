#include "met/autosavedonescreen.h"

#include "met/metagame.h"

AutoSaveDoneScreen::AutoSaveDoneScreen(DataArray *pData) : FreqScreen(pData) {
}

bool AutoSaveDoneScreen::HandleTransitionComplete([[maybe_unused]] UITransitionCompleteMsg *pMsg) {
    TheMetagame.AdvanceUnlocks();
    return false;
}

bool AutoSaveDoneScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
