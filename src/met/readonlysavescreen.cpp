#include "met/readonlysavescreen.h"

#include <cstring>

#include "met/metagame.h"
#include "met/saveremixscreen.h"
#include "met/transitionerrorscreen.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kGuestLaunchpad[] = "fn_g_lpad";
constexpr char kErrorScreen[] = "lpad_error";

} // namespace

ReadOnlySaveScreen::ReadOnlySaveScreen(DataArray *pData) : ErrorScreen(pData) {
}

bool ReadOnlySaveScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool ReadOnlySaveScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }
    const char *pszNext;
    if (strcmp(pMsg->mComponent->mName, "no") == 0) {
        pszNext = kGuestLaunchpad;
    } else {
        pszNext = "save_remix";
        auto *pSave = dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen(pszNext, false));
        pSave->SetStartScreen(kGuestLaunchpad);
        pSave->SetDoneScreen(kGuestLaunchpad);
        pSave->mReservedA0 = 0;
        if (TheNetLaunchpad == nullptr) {
            if (TheMetagame.mNetScreenPending != 0) {
                pSave->SetDoneScreen(TheMetagame.mNetScreen.c_str());
            } else {
                String text(TheLocale.Localize("net_lpad_lost_error_msg", true));
                auto *pError =
                    dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
                pError->mMessage = text.c_str();
                pSave->SetDoneScreen(kErrorScreen);
            }
        }
    }
    TheUI.GotoScreen(pszNext);
    return UIScreen::HandleSelect(pMsg);
}
