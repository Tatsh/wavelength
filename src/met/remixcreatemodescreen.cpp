#include "met/remixcreatemodescreen.h"

#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

RemixCreateModeScreen::RemixCreateModeScreen(DataArray *pData) : FreqScreen(pData) {
}

bool RemixCreateModeScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RemixCreateModeScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }
    String button(pMsg->mComponent->mName);
    TheGameDb->SetRemixActive(button == "modify" ? 0 : 1);
    TheUI.GotoScreen("remix_controller");
    return true;
}

bool RemixCreateModeScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle) {
        if (TheGameDb->mCommunity == GameDb::kCommunityLocal) {
            TheUI.GotoScreen("m_r_sel_song");
        } else {
            TheUI.GotoScreen("s_r_sel_song");
        }
    }
    return false;
}
