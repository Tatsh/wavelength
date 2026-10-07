#include "met/netreadonlywarnscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

NetReadOnlyWarnScreen::NetReadOnlyWarnScreen(DataArray *pData) : ErrorScreen(pData) {
}

bool NetReadOnlyWarnScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetReadOnlyWarnScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        if (strcmp(pMsg->mComponent->mName, "yes") == 0) {
            RemixInfo record = *TheGameDb->GetRemixInfo();
            record.mReadOnly = true;
            TheGameDb->SetRemix(&record);
            TheGameDb->SetRemixReadOnly(1);
            TheUI.GotoScreen("load_remix");
        } else {
            TheUI.GotoScreen("net_remix_read_only_check");
        }
    }
    return UIScreen::HandleSelect(pMsg);
}
