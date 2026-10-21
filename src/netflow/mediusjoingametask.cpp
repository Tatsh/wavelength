#include "netflow/mediusjoingametask.h"

#include <cstring>

#include "msg/launchpadabortedmsg.h"
#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"

void MediusJoinGameTask::OnStart() {
    rtMediusJoinGameRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.messageId);
    request.gameHostType = LaunchpadRT::kGameHostPeerToPeer;
    request.worldId = TheLaunchpadRT->GetWorldId();
    request.password[0] = '\0';
    if (rtMediusJoinGame(&request, OnResponse) != 0) {
        Finish(false, LaunchpadAbortedMsg::kReasonInit);
    }
}

void MediusJoinGameTask::OnResponse(int nConnection,
                                    int nType,
                                    int nSize,
                                    const rtMediusJoinGameResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    int nReason;
    if (pResponse->header.status == kStatusDenied) {
        nReason = LaunchpadAbortedMsg::kReasonDenied;
    } else if (pResponse->header.status != RT_MEDIUS_STATUS_SUCCESS) {
        nReason = LaunchpadAbortedMsg::kReasonConnectFailed;
    } else {
        TheLaunchpadRT->SetConnectionInfo(pResponse->info);
        nReason = 0;
    }
    pTask->Finish(pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS, nReason);
}
