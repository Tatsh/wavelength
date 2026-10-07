#include "met/bootscreen.h"

#include <cstring>

#include "met/dialogpanel.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kLaunchpadScreen[] = "fn_h_lpad";

} // namespace

BootScreen::BootScreen(DataArray *pData) : FreqScreen(pData) {
}

void BootScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    dynamic_cast<DialogPanel *>(mFocusPanel)
        ->SetText(
            FormatString(TheLocale.Localize("net_boot_check_dlg", true), mPlayerName.c_str()));
}

bool BootScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool BootScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszButton = pMsg->mComponent->mName;
        if (strcmp(pszButton, "yes") == 0) {
            if (TheNetLaunchpad != nullptr) {
                TheNetLaunchpad->BootPlayer(mPlayer);
            }
            TheUI.GotoScreen(kLaunchpadScreen);
        } else if (strcmp(pszButton, "no") == 0) {
            TheUI.GotoScreen(kLaunchpadScreen);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}
