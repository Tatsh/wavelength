#include "met/netmainscreen.h"

#include <cstring>

#include "met/chatpanel.h"
#include "met/helppanel.h"
#include "met/metagame.h"
#include "met/netmainlaunchpadspanel.h"
#include "met/netmainpanel.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kL1PanelTag[] = "L1_panel";
constexpr char kMainPanel[] = "fn_main";
constexpr char kChatPanel[] = "fn_main_c";
constexpr char kHelpPanel[] = "help";
constexpr char kJoinComponent[] = "join";
constexpr char kLobbiesComponent[] = "lobbies";
constexpr char kPlayersComponent[] = "players";
constexpr char kLobbiesScreen[] = "fn_main_lobbies";
constexpr char kPlayersScreen[] = "fn_main_players";
constexpr char kJoinScreen[] = "fn_main_join";
constexpr char kLobbiesPanel[] = "fn_main_lob";
constexpr char kPlayersPanel[] = "fn_main_play";
constexpr char kJoinPanel[] = "fn_main_join";
constexpr char kTabScreenFormat[] = "fn_main_%s";
constexpr char kLeaveScreen[] = "netlobby2netwelcome";
constexpr char kTitleToken[] = "fn_main_TITLE";
constexpr char kLobbiesFocusHelpToken[] = "fn_main_lobbies_focus_HELP";
constexpr char kLobbiesHelpToken[] = "fn_main_lobbies_HELP";
constexpr char kNoPanel[] = "";

bool IsNamed(const char *pszName, const char *pszExpected) {
    return std::strcmp(pszName, pszExpected) == 0;
}

} // namespace

NetMainScreen::NetMainScreen(DataArray *pData) : FreqScreen(pData) {
    pData->FindString(kL1PanelTag, &mL1Panel, false);
}

void NetMainScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    if (pPrevScreen != nullptr) {
        mForceEntryExit = dynamic_cast<NetMainScreen *>(pPrevScreen) != nullptr;
    }
    FreqScreen::Enter(pPrevScreen, fTime);
    UIPanel *pFocus = mFocusPanel;
    if (pFocus->mFocus == nullptr) {
        pFocus->SetFocus(TheUI.FindComponent(kMainPanel, kJoinComponent, false), kPadNone);
    }
    static_cast<HelpPanel *>(TheUI.FindPanel(kHelpPanel, false))->ShowHelp(pFocus, pFocus->mFocus);
}

void NetMainScreen::Exit(UIScreen *pNextScreen, float fTime) {
    if (pNextScreen != nullptr && dynamic_cast<NetMainScreen *>(pNextScreen) != nullptr) {
        mForceEntryExit = true;
    } else {
        mForceEntryExit = false;
        if (mFocusPanel != nullptr) {
            mFocusPanel->SetFocus(nullptr, kPadNone);
        }
        SetFocus(TheUI.FindPanel(kMainPanel, false));
    }
    FreqScreen::Exit(pNextScreen, fTime);
}

const char *NetMainScreen::Title() {
    return FormatString(TheLocale.Localize(kTitleToken, true), TheMetagame.mChatroom.mName.c_str());
}

bool NetMainScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nKeyboardKeyMsgType) {
        return HandleKeyboardKey(static_cast<KeyboardKeyMsg *>(pMsg));
    }
    if (nType == g_nLobbyPlayersMsgType) {
        return HandleLobbyPlayers(static_cast<LobbyPlayersMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetMainScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (!IsNamed(pMsg->mPanel->mName, kMainPanel)) {
        return false;
    }
    UIComponent *pTab = pMsg->mComponent;
    if (pTab == nullptr || pTab == pMsg->mOldComponent) {
        return false;
    }
    UIScreen *pScreen = TheUI.FindScreen(FormatString(kTabScreenFormat, pTab->mName), false);
    if (pScreen != nullptr && pScreen != TheUI.mCurrentScreen) {
        TheUI.GotoScreen(pScreen);
    }
    return false;
}

bool NetMainScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return false;
    }
    if (!IsNamed(pMsg->mPanel->mName, kMainPanel)) {
        return false;
    }
    const char *pszTab = pMsg->mComponent->mName;
    if (IsNamed(pszTab, kLobbiesComponent) && IsNamed(kLobbiesScreen, mName)) {
        TheMetagame.SetHelpText(TheLocale.Localize(kLobbiesFocusHelpToken, true));
        SetFocus(TheUI.FindPanel(kLobbiesPanel, false));
    } else if (IsNamed(pszTab, kPlayersComponent) && IsNamed(kPlayersScreen, mName)) {
        SetFocus(TheUI.FindPanel(kPlayersPanel, false));
    } else if (IsNamed(pszTab, kJoinComponent) && IsNamed(kJoinScreen, mName)) {
        SetFocus(TheUI.FindPanel(kJoinPanel, false));
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetMainScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mButton == kPadDRight && IsNamed(pMsg->mPanel->mName, kMainPanel)) {
        const char *pszTab = pMsg->mComponent->mName;
        if (IsNamed(pszTab, kLobbiesComponent)) {
            SetFocus(TheUI.FindPanel(kLobbiesPanel, false));
        } else if (IsNamed(pszTab, kPlayersComponent)) {
            SetFocus(TheUI.FindPanel(kPlayersPanel, false));
        } else if (IsNamed(pszTab, kJoinComponent)) {
            SetFocus(TheUI.FindPanel(kJoinPanel, false));
        }
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool NetMainScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        UIPanel *pFocus = TheUI.FocusPanel();
        const int nButton = pMsg->mButton;
        if (nButton == kPadTriangle && IsNamed(pFocus->mName, kMainPanel)) {
            TheUI.GotoScreen(kLeaveScreen);
        } else if (nButton == kPadTriangle && IsNamed(pFocus->mName, kLobbiesScreen)) {
            TheMetagame.SetHelpText(TheLocale.Localize(kLobbiesHelpToken, true));
        } else if (nButton == kPadCircle) {
            TheUI.mCurrentScreen->SetFocus(TheUI.FindPanel(kMainPanel, false));
            TheUI.FindPanel(kChatPanel, false)->Dispatch(pMsg);
        } else if (nButton == kPadL1 && IsNamed(pFocus->mName, kMainPanel) &&
                   !IsNamed(mL1Panel.c_str(), kNoPanel)) {
            static_cast<NetMainPanel *>(TheUI.FindPanel(mL1Panel.c_str(), false))
                ->RequestNow(false);
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool NetMainScreen::HandleKeyboardKey(KeyboardKeyMsg *pMsg) {
    return TheUI.FindPanel(kChatPanel, false)->Dispatch(pMsg);
}

bool NetMainScreen::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    return static_cast<ChatPanel *>(TheUI.FindPanel(kChatPanel, false))
        ->HandleTextEntryComplete(pMsg);
}

bool NetMainScreen::HandleLobbyPlayers(LobbyPlayersMsg *pMsg) {
    UIScreen *pScreen = TheUI.mCurrentScreen;
    if (pScreen != nullptr && IsNamed(pScreen->mName, kJoinScreen) &&
        IsNamed(pScreen->mFocusPanel->mName, kJoinPanel)) {
        static_cast<NetMainLaunchpadsPanel *>(TheUI.FindPanel(kJoinPanel, false))
            ->ShowPlayers(pMsg->mPlayers, pMsg->mLaunchpad);
    }
    return false;
}
