#include "met/prelaunchscreen.h"

#include "ui/uimanager.h"

PreLaunchScreen::PreLaunchScreen(DataArray *pData) : TransitionScreen(pData) {
    pData->FindSymbol("launch_name", &mLaunchName, false);
}

bool PreLaunchScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (pMsg->mScreen == this) {
        TheUI.GotoScreen(mLaunchName);
    }
    return false;
}

bool PreLaunchScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
