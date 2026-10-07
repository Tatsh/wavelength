#include "met/freqmakererrorscreen.h"

#include <string.h>

#include "met/freqmakerundoscreen.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

FreqMakerErrorScreen::FreqMakerErrorScreen(DataArray *pData) : FreqScreen(pData) {
    mContinueScreen = nullptr;
    mCancelScreen = nullptr;
}

void FreqMakerErrorScreen::SetScreens(const char *pszContinue, const char *pszCancel) {
    mContinueScreen = TheUI.FindScreen(pszContinue, false);
    mCancelScreen = TheUI.FindScreen(pszCancel, false);
}

bool FreqMakerErrorScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszButton = pMsg->mComponent->mName;
    if (strcmp(pszButton, "continue") == 0) {
        dynamic_cast<FreqMakerUndoScreen *>(mCancelScreen)->Undo();
        TheUI.GotoScreen(mContinueScreen);
    } else if (strcmp(pszButton, "cancel") == 0) {
        TheUI.GotoScreen(mCancelScreen);
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool FreqMakerErrorScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
