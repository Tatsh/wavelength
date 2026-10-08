#include "met/netinetconnect.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/dialogpanel.h"
#include "met/transitionerrorscreen.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netinet.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uimanager.h"

namespace {

constexpr char kLobbyScreen[] = "net_connect_lobby";
constexpr char kErrorScreen[] = "net_connect_internet_error";
constexpr char kDisconnectScreen[] = "disconnect_internet";

constexpr char kConfigNotFoundError[] = "inet_error_cfg_not_found";
constexpr char kConfigWrongConsoleError[] = "inet_error_cfg_wrong_console";
constexpr char kConfigError[] = "inet_error_cfg_error";
constexpr char kHardwareError[] = "inet_error_hardware_error";
constexpr char kTimeoutError[] = "inet_error_timeout";
constexpr char kGenericError[] = "net_connect_internet_error";

// The memory card slot of the configuration.
constexpr int kConfigSlot = 0;

// The operation of ErrorScreen::ShowCardErrorTwoOption() for a failure outside a save, a copy, and
// a deletion.
constexpr int kNoOperation = 0;

const char *ErrorToken(int nResult) {
    switch (nResult) {
    case InetConnectResultMsg::kResultConfigNotFound:
        return kConfigNotFoundError;
    case InetConnectResultMsg::kResultConfigWrongConsole:
        return kConfigWrongConsoleError;
    case InetConnectResultMsg::kResultConfigError:
        return kConfigError;
    case InetConnectResultMsg::kResultHardwareError:
        return kHardwareError;
    case InetConnectResultMsg::kResultTimeout:
        return kTimeoutError;
    default:
        return kGenericError;
    }
}

} // namespace

NetInetConnect::NetInetConnect(DataArray *pData) : ErrorScreen(pData), mConnecting(0) {
}

void NetInetConnect::OnCardStatus(int nStatus) {
    if (nStatus == MemcardTask::kStatusOk) {
        mConnecting = 1;
        TheNetInet->Connect(this, mConfig);
    } else {
        ShowCardErrorTwoOption(nStatus, kNoOperation);
    }
}

bool NetInetConnect::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nInetConnectResultMsgType) {
        return HandleResult(static_cast<InetConnectResultMsg *>(pMsg));
    }
    if (nType == g_nInetConnectStatusMsgType) {
        return HandleStatus(static_cast<InetConnectStatusMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetInetConnect::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.GetCardStatus(this, kConfigSlot);
    }
    return false;
}

bool NetInetConnect::HandleStatus(InetConnectStatusMsg *pMsg) {
    DialogPanel *pDialog =
        mFocusPanel != nullptr ? dynamic_cast<DialogPanel *>(mFocusPanel) : nullptr;
    pDialog->SetText(pMsg->mStatus.c_str());
    return false;
}

bool NetInetConnect::HandleResult(InetConnectResultMsg *pMsg) {
    mConnecting = 0;
    if (pMsg->mResult == InetConnectResultMsg::kResultSuccess) {
        TheUI.GotoScreen(kLobbyScreen);
        return false;
    }

    String message(TheLocale.Localize(ErrorToken(pMsg->mResult), true));
    auto *pError = dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kErrorScreen, false));
    pError->mMessage = message.c_str();
    TheUI.GotoScreen(pError);
    return false;
}

bool NetInetConnect::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle && mConnecting != 0) {
        mConnecting = 0;
        TheUI.GotoScreen(kDisconnectScreen);
    }
    return false;
}
