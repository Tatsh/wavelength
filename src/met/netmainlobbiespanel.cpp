#include "met/netmainlobbiespanel.h"

#include <cstring>

#include "met/metagame.h"
#include "met/netmainlobbieslist.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/system.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uiscreen.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kCursorComponent[] = "cursor";
constexpr char kMainPanel[] = "fn_main";
constexpr char kSwitchScreen[] = "net_switch_lobby_confirm";
constexpr char kNewChatroomScreen[] = "fn_new_lob";
constexpr char kFocusHelpToken[] = "fn_main_lobbies_focus_HELP";
constexpr char kHelpToken[] = "fn_main_lobbies_HELP";

// The milliseconds after a reply before the chat rooms are requested again.
constexpr float kRequestIntervalMs = 20000.0f;

NetMainLobbiesList *FindList(UIPanel *pPanel) {
    return static_cast<NetMainLobbiesList *>(pPanel->FindComponent(kListComponent, false));
}

} // namespace

void NetMainLobbiesPanel::RequestUpdate() {
    TheNetLobby->RequestChatrooms(this);
}

void NetMainLobbiesPanel::Focus() {
    if (!mLoaded) {
        return;
    }
    FocusChangePanel::Focus();
    mHilite = 1;
    FindList(this)->SetCursorSelected(true);
    TheMetagame.SetHelpText(TheLocale.Localize(kFocusHelpToken, true));
}

void NetMainLobbiesPanel::Unfocus() {
    if (!mLoaded) {
        return;
    }
    FocusChangePanel::Unfocus();
    mHilite = 0;
    FindList(this)->SetCursorSelected(false);
    TheMetagame.SetHelpText(TheLocale.Localize(kHelpToken, true));
}

bool NetMainLobbiesPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLobbyChatroomsMsgType) {
        return HandleChatrooms(static_cast<LobbyChatroomsMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return NetMainPanel::DispatchPriv(pMsg);
}

bool NetMainLobbiesPanel::HandleChatrooms(LobbyChatroomsMsg *pMsg) {
    if (mLoaded) {
        FindList(this)->SetChatrooms(pMsg->mChatrooms);
        HideFlash();
    }
    mRequestTime = SystemMs() + kRequestIntervalMs;
    return false;
}

bool NetMainLobbiesPanel::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross && std::strcmp(pMsg->mComponent->mName, kCursorComponent) == 0) {
        TheUI.mCurrentScreen->SetFocus(TheUI.FindPanel(kMainPanel, false));
        TheUI.GotoScreen(kSwitchScreen);
    }
    return false;
}

bool NetMainLobbiesPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        UIScreen *pScreen = TheUI.mCurrentScreen;
        if (pScreen->mNextScreen != nullptr || pScreen->mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadSquare) {
            FxMidi::PlaySquare();
            TheUI.mCurrentScreen->SetFocus(TheUI.FindPanel(kMainPanel, false));
            TheUI.GotoScreen(kNewChatroomScreen);
        }
    }
    return NetMainPanel::HandleJoypad(pMsg);
}
