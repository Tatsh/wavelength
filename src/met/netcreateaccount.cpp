#include "met/netcreateaccount.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/netserverlogin.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "ui/uimanager.h"

namespace {

constexpr char kServerLoginScreen[] = "net_server_login";
constexpr char kAlreadyExistsScreen[] = "net_account_already_exists_error";
constexpr char kRegistrationFailedScreen[] = "net_registration_failed_error";
constexpr char kVulgarNameScreen[] = "net_registration_vulgar_error";
constexpr char kCreateErrorScreen[] = "net_create_account_error";
constexpr char kDisconnectLobbyScreen[] = "net_disconnect_lobby";

constexpr int kLocalPlayer = 0;
constexpr int kNameLocked = 1;
constexpr int kSaveChanged = 1;

} // namespace

NetCreateAccount::NetCreateAccount(DataArray *pData) : FreqScreen(pData), mPassword("") {
}

bool NetCreateAccount::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nCreateAccountResultMsgType) {
        return HandleCreateAccountResult(static_cast<CreateAccountResultMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetCreateAccount::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLobby->CreateAccount(this, mPassword.c_str());
    }
    return false;
}

bool NetCreateAccount::HandleCreateAccountResult(CreateAccountResultMsg *pMsg) {
    switch (pMsg->mResult) {
    case CreateAccountResultMsg::kResultSuccess: {
        NetServerLogin *pLogin =
            static_cast<NetServerLogin *>(TheUI.FindScreen(kServerLoginScreen, false));
        pLogin->mPassword = mPassword.c_str();
        TheGameDb->GetProfile(kLocalPlayer)->mNameLocked = kNameLocked;
        pLogin->mSaveChanged = kSaveChanged;
        pLogin->mSavePassword = mSavePassword;
        TheUI.GotoScreen(pLogin);
        break;
    }
    case CreateAccountResultMsg::kResultAlreadyExists:
        TheUI.GotoScreen(kAlreadyExistsScreen);
        break;
    case CreateAccountResultMsg::kResultRegistrationFailed:
        TheUI.GotoScreen(kRegistrationFailedScreen);
        break;
    case CreateAccountResultMsg::kResultVulgarName:
        TheUI.GotoScreen(kVulgarNameScreen);
        break;
    default:
        TheUI.GotoScreen(kCreateErrorScreen);
        break;
    }
    return false;
}

bool NetCreateAccount::HandleJoypad(JoypadInputMsg *pMsg) {
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
