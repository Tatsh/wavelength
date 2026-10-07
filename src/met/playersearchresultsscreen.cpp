#include "met/playersearchresultsscreen.h"

#include <cstring>

#include "met/metagame.h"
#include "met/netfoundplayerpanel.h"
#include "met/netswitchlobbyscreen.h"
#include "os/joypad.h"
#include "ui/uibutton.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kFoundPlayerPanel[] = "fn_find_p";
constexpr char kNameLabel[] = "freq_name";
constexpr char kChatroomLabel[] = "lobby_name";
constexpr char kBackButton[] = "back";
constexpr char kChatroomButton[] = "new";
constexpr char kLobbyScreen[] = "fn_main_join";
constexpr char kSwitchLobbyScreen[] = "net_switch_lobby";

} // namespace

PlayerSearchResultsScreen::PlayerSearchResultsScreen(DataArray *pData) : FreqScreen(pData) {
}

void PlayerSearchResultsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    NetFoundPlayerPanel *pPanel =
        static_cast<NetFoundPlayerPanel *>(TheUI.FindPanel(kFoundPlayerPanel, false));
    pPanel->SetPlayer(mPlayer, mChatroomId, mChatroomName.c_str());
    FreqScreen::Enter(pPrevScreen, fTime);

    const String unused; // Yes, the binary builds and discards this string.
    dynamic_cast<UILabel *>(TheUI.FindComponent(mFocusPanel->mName, kNameLabel, false))
        ->SetText(mPlayer.mName.c_str());
    if (mInChatroom != 0) {
        dynamic_cast<UILabel *>(TheUI.FindComponent(mFocusPanel->mName, kChatroomLabel, false))
            ->SetText(mChatroomName.c_str());
    }

    UIButton *pBack =
        dynamic_cast<UIButton *>(TheUI.FindComponent(mFocusPanel->mName, kBackButton, false));
    pBack->SetText(TheMetagame.mChatroom.mName.c_str());
    if (mInChatroom != 0) {
        dynamic_cast<UIButton *>(TheUI.FindComponent(mFocusPanel->mName, kChatroomButton, false))
            ->SetText(mChatroomName.c_str());
    } else {
        pBack->SetState(UIComponent::kStateSelected, true);
    }
}

bool PlayerSearchResultsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool PlayerSearchResultsScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        if (std::strcmp(pMsg->mComponent->mName, kBackButton) == 0) {
            TheUI.GotoScreen(kLobbyScreen);
        } else {
            NetSwitchLobbyScreen *pSwitch =
                dynamic_cast<NetSwitchLobbyScreen *>(TheUI.FindScreen(kSwitchLobbyScreen, false));
            pSwitch->mChatroomId = mChatroomId;
            TheUI.GotoScreen(pSwitch);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}
