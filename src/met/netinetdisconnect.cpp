#include "met/netinetdisconnect.h"

#include <cstring>

#include "netflow/netinet.h"
#include "ui/uimanager.h"

namespace {

constexpr char kConfigScreen[] = "fn_config";

} // namespace

NetInetDisconnect::NetInetDisconnect(DataArray *pData) : FreqScreen(pData) {
}

bool NetInetDisconnect::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nInetDisconnectResultMsgType) {
        return HandleDisconnectResult(static_cast<InetDisconnectResultMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetInetDisconnect::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetInet->SetSink(this);
    }
    return false;
}

bool NetInetDisconnect::HandleDisconnectResult([[maybe_unused]] InetDisconnectResultMsg *pMsg) {
    TheUI.GotoScreen(kConfigScreen);
    return false;
}
