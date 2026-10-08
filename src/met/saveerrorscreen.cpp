#include "met/saveerrorscreen.h"

#include <cstring>

#include "met/overwritesavescreen.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kReplaceComponent[] = "replace";

// The value of OverwriteSaveScreen::mOverwriteStatus that allows the replacement.
constexpr int kOverwrite = 1;

} // namespace

bool SaveErrorScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SaveErrorScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross &&
        std::strcmp(pMsg->mComponent->mName, kReplaceComponent) == 0) {
        auto *pScreen =
            dynamic_cast<OverwriteSaveScreen *>(TheUI.FindScreen(mStartScreen.c_str(), false));
        pScreen->mOverwriteStatus = kOverwrite;
        TheUI.GotoScreen(pScreen);
    }
    return UIScreen::HandleSelect(pMsg);
}
