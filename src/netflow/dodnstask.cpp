#include "netflow/dodnstask.h"

#include "msg/lobbyconnectresultmsg.h"
#include "netflow/inetdnslookupresultmsg.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"

void DoDNSTask::OnStart() {
    TheNetInet->LookupHost(this, TheLobbyRT->GetServerName());
}

void DoDNSTask::OnStop() {
    TheNetInet->ClearPending();
}

bool DoDNSTask::OnResult(InetDNSLookupResultMsg *pMsg) {
    Finish(pMsg->GetResult() == 0, LobbyConnectResultMsg::kResultLookupFailed);
    if (pMsg->GetResult() == 0) {
        TheLobbyRT->SetServerAddress(pMsg->GetAddress());
    }
    return false;
}

bool DoDNSTask::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nInetDNSLookupResultMsgType) {
        return OnResult(static_cast<InetDNSLookupResultMsg *>(pMsg));
    }
    return false;
}
