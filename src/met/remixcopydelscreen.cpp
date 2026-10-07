#include "met/remixcopydelscreen.h"

#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "s_r_load";
constexpr char kListComponent[] = "list";

} // namespace

RemixCopyDelScreen::RemixCopyDelScreen(DataArray *pData) : RemixSelectScreen(pData) {
}

bool RemixCopyDelScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return RemixSelectScreen::DispatchPriv(pMsg);
}

bool RemixCopyDelScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    static_cast<UIList *>(TheUI.FindComponent(kPanel, kListComponent, false))
        ->SetCursorSelected(true);
    return UIScreen::HandleSelect(pMsg);
}

bool RemixCopyDelScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    return UIScreen::HandleJoypad(pMsg);
}
