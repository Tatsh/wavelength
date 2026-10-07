#include "met/freqcopydelscreen.h"

#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "f_load";
constexpr char kListComponent[] = "list";

} // namespace

FreqCopyDelScreen::FreqCopyDelScreen(DataArray *pData) : SelLoadedFreqScreen(pData) {
}

bool FreqCopyDelScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return SelLoadedFreqScreen::DispatchPriv(pMsg);
}

void FreqCopyDelScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    SelLoadedFreqScreen::Enter(pPrevScreen, fTime);
}

bool FreqCopyDelScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    static_cast<UIList *>(TheUI.FindComponent(kPanel, kListComponent, false))
        ->SetCursorSelected(true);
    return UIScreen::HandleSelect(pMsg);
}

bool FreqCopyDelScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    return UIScreen::HandleJoypad(pMsg);
}
