#include "met/netchangepassword.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/savefreqscreen.h"
#include "netflow/netlobby.h"
#include "ui/uimanager.h"

namespace {

constexpr char kSaveFreqScreen[] = "login_save_freq";
constexpr char kWelcomeScreen[] = "fn_welcome";
constexpr char kUpdateErrorScreen[] = "net_password_update_error";
constexpr char kChangeErrorScreen[] = "net_password_change_error";
constexpr char kNoPassword[] = "";

constexpr int kLocalPlayer = 0;
constexpr int kOverwrite = 1;

} // namespace

NetChangePassword::NetChangePassword(DataArray *pData) : FreqScreen(pData) {
}

bool NetChangePassword::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nChangePasswordResultMsgType) {
        return HandleChangePasswordResult(static_cast<ChangePasswordResultMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetChangePassword::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->GetScreen()->GetName(), mName) == 0) {
        TheNetLobby->ChangePassword(this, mOldPassword.c_str(), mNewPassword.c_str());
    }
    return false;
}

bool NetChangePassword::HandleChangePasswordResult(ChangePasswordResultMsg *pMsg) {
    if (pMsg->GetResult() == ChangePasswordResultMsg::kResultUpdateFailed) {
        TheUI.GotoScreen(kUpdateErrorScreen);
    } else if (pMsg->GetResult() != ChangePasswordResultMsg::kResultSuccess) {
        TheUI.GotoScreen(kChangeErrorScreen);
    } else if (mSaveChanged == 0) {
        TheUI.GotoScreen(kWelcomeScreen);
    } else {
        if (mSavePassword != 0) {
            TheGameDb->GetProfile(kLocalPlayer)->SetPassword(mNewPassword.c_str());
        } else {
            TheGameDb->GetProfile(kLocalPlayer)->SetPassword(kNoPassword);
        }
        SaveFreqScreen *pSave =
            dynamic_cast<SaveFreqScreen *>(TheUI.FindScreen(kSaveFreqScreen, false));
        pSave->SetOverwriteStatus(kOverwrite);
        TheUI.GotoScreen(pSave);
    }
    return false;
}
