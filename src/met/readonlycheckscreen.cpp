#include "met/readonlycheckscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

ReadOnlyCheckScreen::ReadOnlyCheckScreen(DataArray *pData)
    : FreqScreen(pData), mNeedsShareWarning(false) {
    pData->FindString("next_screen", &mNextScreen, true);
    pData->FindBool("needs_share_warning", &mNeedsShareWarning, false);
}

bool ReadOnlyCheckScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool ReadOnlyCheckScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }
    bool bReadOnly = false;
    if (strcmp(pMsg->mComponent->mName, "no") == 0) {
        if (mNeedsShareWarning) {
            TheUI.GotoScreen("net_share_read_only_warning");
            return true;
        }
        bReadOnly = true;
    }
    RemixInfo record = *TheGameDb->GetRemixInfo();
    record.mReadOnly = bReadOnly;
    TheGameDb->SetRemix(&record);
    TheGameDb->SetRemixReadOnly(bReadOnly);
    TheUI.GotoScreen(mNextScreen.c_str());
    return true;
}
