#include "met/multinocardscreen.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

#include "met/multiloadfreqscreen.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kLoadScreen[] = "multi_load_freq";
constexpr char kRetryComponent[] = "retry";
constexpr char kContinueComponent[] = "continue";

} // namespace

bool MultiNoCardScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool MultiNoCardScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        auto *pLoad = dynamic_cast<MultiLoadFreqScreen *>(TheUI.FindScreen(kLoadScreen, false));
        if (std::strcmp(pMsg->mComponent->mName, kRetryComponent) == 0) {
            TheUI.GotoScreen(pLoad);
        } else if (std::strcmp(pMsg->mComponent->mName, kContinueComponent) == 0) {
            pLoad->mLoaded = 1;
            TheUI.GotoScreen(pLoad);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

#endif
