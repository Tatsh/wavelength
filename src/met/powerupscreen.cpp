#include "met/powerupscreen.h"

#include <string.h>

#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPowerupButton[] = "powerup";

} // namespace

PowerupScreen::PowerupScreen(DataArray *pData) : FreqScreen(pData) {
}

bool PowerupScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void PowerupScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mLabels.clear();
    const char *pszFormat = TheLocale.Localize("NET_HOST_PUP", true);
    mLabels.push_back(String(FormatString(pszFormat, TheLocale.Localize("pup_low", true))));
    mLabels.push_back(String(FormatString(pszFormat, TheLocale.Localize("pup_hi", true))));
    mLabels.push_back(String(FormatString(pszFormat, TheLocale.Localize("pup_off", true))));
    mLevel = TheGameDb->mPowerupLevel;
    UIComponent *pButton = TheUI.FindComponent("m_g_sel_pup", kPowerupButton, false);
    pButton->SetText(mLabels[mLevel].c_str());
    pButton->SetState(UIComponent::kStateSelected, true);
}

bool PowerupScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (strcmp(pMsg->mComponent->mName, kPowerupButton) != 0) {
        return FreqScreen::HandleSelectStart(pMsg);
    }
    const int nCount = static_cast<int>(mLabels.size());
    if (pMsg->mButton == kPadDLeft) {
        mLevel = mLevel - 1 > -1 ? mLevel - 1 : nCount - 1;
    } else if (pMsg->mButton == kPadDRight) {
        mLevel = mLevel + 1 < nCount ? mLevel + 1 : 0;
    }
    UIComponent *pButton = TheUI.FindComponent("m_g_sel_pup", kPowerupButton, false);
    pButton->SetText(mLabels[mLevel].c_str());
    return FreqScreen::HandleSelectStart(pMsg);
}

bool PowerupScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        if (strcmp(pMsg->mComponent->mName, kPowerupButton) == 0) {
            TheGameDb->SetPowerupLevel(mLevel);
        }
        TheUI.GotoScreen("multiskill2multiarena");
    }
    return UIScreen::HandleSelect(pMsg);
}
