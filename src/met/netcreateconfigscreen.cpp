#include "met/netcreateconfigscreen.h"

#include <cstring>

#include "netflow/netinet.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kYesButton[] = "yes";
constexpr char kNoButton[] = "no";
constexpr char kConfigScreen[] = "fn_config";

} // namespace

NetCreateConfigScreen::NetCreateConfigScreen(DataArray *pData) : FreqScreen(pData) {
}

bool NetCreateConfigScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetCreateConfigScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszButton = pMsg->mComponent->mName;
        if (std::strcmp(pszButton, kYesButton) == 0) {
            TheNetInet->LaunchConfigTool();
            return true;
        }
        if (std::strcmp(pszButton, kNoButton) == 0) {
            TheUI.GotoScreen(kConfigScreen);
            return true;
        }
    }
    return UIScreen::HandleSelect(pMsg);
}
