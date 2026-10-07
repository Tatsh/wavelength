#include "met/netlpadscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/chatpanel.h"
#include "met/netjoinlpadscreen.h"
#include "met/netlpadgamepanel.h"
#include "met/netlpadpanel.h"
#include "msg/chatmsg.h"
#include "netflow/netlaunchpad.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kChatPanel[] = "fn_h_lpad_c";
constexpr char kPlayerCommand[] = "player";

// The notices of the session go to every console on this channel, and show here too.
constexpr int kNoticeChannel = 2;
constexpr int kEveryConsole = 0;
constexpr int kEcho = 1;

// IsGuest() reports this value for the host.
constexpr int kHostIndex = 0;

void SendNotice(const char *pszToken) {
    ChatMsg::Send(kNoticeChannel, kEveryConsole, TheLocale.Localize(pszToken, true), kEcho);
}

ChatPanel *FindChatPanel() {
    return static_cast<ChatPanel *>(TheUI.FindPanel(kChatPanel, false));
}

} // namespace

NetLpadScreen::NetLpadScreen(DataArray *pData) : FreqScreen(pData), mPlayersVersion(0) {
    pData->FindSymbol("dataPanel", &mDataPanel, false);
    pData->FindSymbol("buttonPanel", &mButtonPanel, false);
}

void NetLpadScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    if (pPrevScreen != nullptr) {
        mForceEntryExit = dynamic_cast<NetLpadScreen *>(pPrevScreen) != nullptr;
        if (strcmp(pPrevScreen->mName, "load_remix") == 0) {
            SendNotice("host_done_edit_msg");
            if (TheNetLaunchpad != nullptr) {
                TheNetLaunchpad->EndEdit();
            }
        }
    }
    --mPlayersVersion;
    FreqScreen::Enter(pPrevScreen, fTime);
}

void NetLpadScreen::Exit(UIScreen *pNextScreen, float fTime) {
    if (pNextScreen != nullptr && dynamic_cast<NetLpadScreen *>(pNextScreen) != nullptr) {
        mForceEntryExit = true;
    } else {
        mForceEntryExit = false;
        if (mFocusPanel != nullptr) {
            mFocusPanel->SetFocus(nullptr, kPadNone);
        }
        SetFocus(TheUI.FindPanel(mButtonPanel, false));
    }
    FreqScreen::Exit(pNextScreen, fTime);
}

void NetLpadScreen::UpdateGameParams(const NetGameParams *pParams) {
    TheGameDb->SetGameParams(pParams);
    if (strcmp(mName, TheUI.mCurrentScreen->mName) != 0) {
        return;
    }
    auto *pPanel = dynamic_cast<NetLPadGamePanel *>(TheUI.FindPanel(mDataPanel, false));
    if (pPanel != nullptr && pPanel->IsLoaded()) {
        pPanel->RefreshGame();
    }
}

void NetLpadScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (!IsLoaded() || mNextScreen != nullptr) {
        return;
    }
    if (TheNetLaunchpad == nullptr) {
        NetJoinLPadScreen::ShowLaunchpadLost();
        return;
    }
    if (TheNetLaunchpad->GetPlayersVersion() == mPlayersVersion) {
        return;
    }
    mPlayersVersion = TheNetLaunchpad->GetPlayersVersion();
    mPlayers = *TheNetLaunchpad->GetPlayers();
    if (TheNetLaunchpad->IsGuest() == kHostIndex) {
        dynamic_cast<NetLPadPanel *>(TheUI.FindPanel(mButtonPanel, false))->Update(&mPlayers);
    } else {
        UIPanel *pButtons = TheUI.FindPanel(mButtonPanel, false);
        if (pButtons->mFocus == nullptr) {
            pButtons->SetFocus(pButtons->FindComponent("data", false), kPadNone);
        }
    }
    dynamic_cast<NetLPadPanel *>(TheUI.FindPanel(mDataPanel, false))->Update(&mPlayers);
}

bool NetLpadScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
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
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetLpadScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadCircle && pMsg->mPressed != 0) {
        TheUI.mCurrentScreen->SetFocus(TheUI.FindPanel(mButtonPanel, false));
        TheUI.mCurrentScreen->mFocusPanel->SetFocus(nullptr, kPadNone);
        FindChatPanel()->Dispatch(pMsg);
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool NetLpadScreen::HandleKeyboardKey(KeyboardKeyMsg *pMsg) {
    return FindChatPanel()->Dispatch(pMsg);
}

bool NetLpadScreen::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    return FindChatPanel()->HandleTextEntryComplete(pMsg);
}

bool NetLpadScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (strcmp(pMsg->mPanel->mName, mButtonPanel) != 0) {
        return false;
    }
    UIComponent *pComponent = pMsg->mComponent;
    if (pComponent == nullptr || pComponent == pMsg->mOldComponent) {
        return false;
    }
    const char *pszScreen = mButtonPanel;
    if (strcmp(pComponent->mName, kPlayerCommand) == 0) {
        pszScreen = FormatString("%s_play", mButtonPanel);
    }
    UIScreen *pScreen = TheUI.FindScreen(pszScreen, false);
    if (pScreen != nullptr && pScreen != TheUI.mCurrentScreen) {
        TheUI.GotoScreen(pScreen);
    }
    return false;
}

bool NetLpadScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mButton == kPadDRight && strcmp(pMsg->mComponent->mName, kPlayerCommand) == 0) {
        const char *pszPanel = pMsg->mPanel->mName;
        if (strcmp(pszPanel, "fn_g_lpad") == 0 || strcmp(pszPanel, "fn_h_lpad") == 0) {
            SetFocus(TheUI.FindPanel(mDataPanel, false));
        }
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool NetLpadScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }
    const char *pszCommand = pMsg->mComponent->mName;
    if (TheNetLaunchpad != nullptr && strcmp(pszCommand, "launch") == 0) {
        TheUI.GotoScreen("net_launch");
    } else if (strcmp(pszCommand, "edit") == 0) {
        SendNotice("host_edit_msg");
        TheUI.GotoScreen("edit_host");
    } else if (TheNetLaunchpad != nullptr && strcmp(pszCommand, "share") == 0) {
        DebugPrint("asking launchpad to shareRemix\n");
        if (TheNetLaunchpad->ShareRemix()) {
            TheUI.GotoScreen("net_share_remix");
        }
    } else if (strcmp(pszCommand, kPlayerCommand) == 0) {
        if (strcmp(mFocusPanel->mName, mButtonPanel) == 0) {
            SetFocus(TheUI.FindPanel(mDataPanel, false));
        }
    } else if (strcmp(pszCommand, "abort") == 0) {
        TheUI.GotoScreen("net_launchpad_quit");
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetLpadScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (TheNetLaunchpad != nullptr && strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLaunchpad->Activate();
    } else {
        NetJoinLPadScreen::ShowLaunchpadLost();
    }
    return false;
}
