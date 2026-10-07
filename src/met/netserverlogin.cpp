#include "met/netserverlogin.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/chatpanel.h"
#include "met/metagame.h"
#include "met/neteulascreen.h"
#include "met/netwelcomescreen.h"
#include "met/savefreqscreen.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kChatPanel[] = "fn_main_c";
constexpr char kWelcomeScreen[] = "fn_welcome";
constexpr char kEulaScreen[] = "d_eula";
constexpr char kSaveFreqScreen[] = "login_save_freq";
constexpr char kDisconnectLobbyScreen[] = "net_disconnect_lobby";
constexpr char kAccountNotFoundScreen[] = "net_account_not_found_error";
constexpr char kInvalidPasswordScreen[] = "net_invalid_password_error";
constexpr char kAlreadyLoggedInScreen[] = "net_already_logged_in_error";
constexpr char kServerLoginErrorScreen[] = "net_server_login_error";
constexpr char kNoPassword[] = "";

constexpr int kLocalPlayer = 0;
constexpr int kNameLocked = 1;
constexpr int kNameUnlocked = 0;
constexpr int kOverwrite = 1;

} // namespace

NetServerLogin::NetServerLogin(DataArray *pData)
    : FreqScreen(pData), mLocation(0), mPassword(kNoPassword) {
}

bool NetServerLogin::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nLoginResultMsgType) {
        return HandleLoginResult(static_cast<LoginResultMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetServerLogin::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLobby->Login(this, mLocation, mPassword.c_str());
    }
    return false;
}

bool NetServerLogin::HandleLoginResult(LoginResultMsg *pMsg) {
    switch (pMsg->mResult) {
    case LoginResultMsg::kResultSuccess:
        break;
    case LoginResultMsg::kResultAccountNotFound:
        TheGameDb->GetProfile(kLocalPlayer)->mNameLocked = kNameUnlocked;
        TheUI.GotoScreen(kAccountNotFoundScreen);
        return false;
    case LoginResultMsg::kResultInvalidPassword:
        TheUI.GotoScreen(kInvalidPasswordScreen);
        return false;
    case LoginResultMsg::kResultAlreadyLoggedIn:
        TheUI.GotoScreen(kAlreadyLoggedInScreen);
        return false;
    default:
        TheUI.GotoScreen(kServerLoginErrorScreen);
        return false;
    }

    dynamic_cast<ChatPanel *>(TheUI.FindPanel(kChatPanel, false))->mMessages.clear();
    TheMetagame.mChatroom = pMsg->mChatroom;
    dynamic_cast<NetWelcomeScreen *>(TheUI.FindScreen(kWelcomeScreen, false))->mNews = pMsg->mNews;
    dynamic_cast<NetEULAScreen *>(TheUI.FindScreen(kEulaScreen, false))->SetText(pMsg->mEula);

    if (mSaveChanged == 0 && TheGameDb->GetProfile(kLocalPlayer)->mNameLocked == kNameLocked) {
        TheUI.GotoScreen(kEulaScreen);
        return false;
    }
    if (mSavePassword != 0) {
        TheGameDb->GetProfile(kLocalPlayer)->mPassword = mPassword.c_str();
    } else {
        TheGameDb->GetProfile(kLocalPlayer)->mPassword = kNoPassword;
    }
    TheGameDb->GetProfile(kLocalPlayer)->mNameLocked = kNameLocked;
    SaveFreqScreen *pSave =
        dynamic_cast<SaveFreqScreen *>(TheUI.FindScreen(kSaveFreqScreen, false));
    pSave->mOverwriteStatus = kOverwrite;
    TheUI.GotoScreen(pSave);
    return false;
}

bool NetServerLogin::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle) {
        TheUI.GotoScreen(kDisconnectLobbyScreen);
    }
    return false;
}
