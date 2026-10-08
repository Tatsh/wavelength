#include "met/numplayersscreen.h"

#include "game/campaign.h"
#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kPanel[] = "m_player";
constexpr char kTwoPlayersButton[] = "2play";
constexpr char kThreePlayersButton[] = "3play";
constexpr char kFourPlayersButton[] = "4play";

} // namespace

NumPlayersScreen::NumPlayersScreen(DataArray *pData) : FreqScreen(pData), mMultitap(1) {
}

void NumPlayersScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    UpdateButtons(JoypadMultitapConnected());
}

void NumPlayersScreen::UpdateButtons(int nMultitap) {
    mMultitap = nMultitap;
    UIPanel *pPanel = TheUI.FindPanel(kPanel, false);
    pPanel->SetFocus(nullptr, kPadNone);
    const int nState = nMultitap != 0 ? UIComponent::kStateNormal : UIComponent::kStateDisabled;
    TheUI.FindComponent(kPanel, kThreePlayersButton, false)->SetState(nState, false);
    TheUI.FindComponent(kPanel, kFourPlayersButton, false)->SetState(nState, false);
    pPanel->SetFocus(TheUI.FindComponent(kPanel, kTwoPlayersButton, false), kPadNone);
}

void NumPlayersScreen::Poll(float fTime) {
    FreqScreen::Poll(fTime);
    const int nMultitap = JoypadMultitapConnected();
    if (nMultitap != mMultitap) {
        UpdateButtons(nMultitap);
    }
}

bool NumPlayersScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NumPlayersScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    String button(pMsg->mComponent->mName);
    if (TheGameDb->GetProfile(0)->mCustom != 0) {
        Campaign first(*TheGameDb->GetProfile(0));
        TheGameDb->ClearPlayers();
        TheGameDb->AddPlayer(&first);
    } else {
        TheGameDb->ClearPlayers();
        Campaign first;
        first.mName = TheLocale.Localize("player_1", true);
        TheGameDb->AddPlayer(&first);
    }

    // The binary renames one profile for each further player.
    Campaign other;
    other.mName = TheLocale.Localize("player_2", true);
    TheGameDb->AddPlayer(&other);
    if (button == kThreePlayersButton) {
        other.mName = TheLocale.Localize("player_3", true);
        TheGameDb->AddPlayer(&other);
    } else if (button == kFourPlayersButton) {
        other.mName = TheLocale.Localize("player_3", true);
        TheGameDb->AddPlayer(&other);
        other.mName = TheLocale.Localize("player_4", true);
        TheGameDb->AddPlayer(&other);
    }
    return UIScreen::HandleSelect(pMsg);
}
