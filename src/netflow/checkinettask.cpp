#include "netflow/checkinettask.h"

#include "msg/lobbyconnectresultmsg.h"
#include "netflow/inetcheckconnectionresultmsg.h"
#include "netflow/netinet.h"

void CheckInetTask::OnStart() {
    TheNetInet->CheckConnection(this);
}

bool CheckInetTask::OnResult(InetCheckConnectionResultMsg *pMsg) {
    Finish(pMsg->GetResult() == 0, LobbyConnectResultMsg::kResultLostInternet);
    return false;
}

bool CheckInetTask::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nInetCheckConnectionResultMsgType) {
        return OnResult(static_cast<InetCheckConnectionResultMsg *>(pMsg));
    }
    return false;
}
