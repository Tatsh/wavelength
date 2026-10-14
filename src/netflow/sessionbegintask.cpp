#include "netflow/sessionbegintask.h"

#include <cstring>

#include "msg/lobbyconnectresultmsg.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"

void SessionBeginTask::OnStart() {
    rtMediusSessionBeginRequest request;
    memset(&request, 0, sizeof(request));
    request.connectionClass = LobbyRT::ConnectionClassOf(TheNetInet->GetConnectionType());
    MakeTag(this, request.messageId);
    const int nError = rtMediusSessionBegin(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, LobbyConnectResultMsg::kResultSessionFailed);
    }
}

void SessionBeginTask::OnResponse(int nConnection,
                                  int nType,
                                  int nSize,
                                  const rtMediusSessionBeginResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status >= RT_MEDIUS_STATUS_SUCCESS) {
        TheLobbyRT->mSessionKey = pResponse->sessionKey;
    } else {
        TheLobbyRT->mSessionKey = "";
    }
    pTask->Finish(pResponse->header.status >= RT_MEDIUS_STATUS_SUCCESS,
                  LobbyConnectResultMsg::kResultSessionFailed);
}
