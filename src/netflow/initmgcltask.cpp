#include "netflow/initmgcltask.h"

#include <cstring>

#include "msg/launchpadabortedmsg.h"
#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"

namespace {

// The size of the buffer the version of the library is copied into.
constexpr int kVersionBufferSize = 128;

} // namespace

void InitMGCLTask::OnStart() {
    rtMgclInitParams params;
    memset(&params, 0, sizeof(params));
    params.handlers[0] = OnServerRequest0;
    params.handlers[1] = OnServerRequest1;
    params.handlers[2] = OnServerRequest2;
    params.handlers[3] = OnServerRequest3;
    params.maxClients = LaunchpadRT::kMaxClients;
    params.applicationId = LobbyRT::kApplicationId;
    params.publicKey = nullptr;
    params.connectMode = 0;
    int nStatus;
    memset(&nStatus, 0, sizeof(nStatus));
    const int nResult = rtMgclInitialize(&params, &nStatus);

    char szVersion[kVersionBufferSize];
    szVersion[0] = '\0';
    (void)rtMgclGetVersion(szVersion); // Yes, the binary discards the version.

    const bool bReady = nResult == 0;
    LaunchpadRT::SetMgclReady(bReady);
    Finish(bReady, LaunchpadAbortedMsg::kReasonInit);
}

int InitMGCLTask::OnServerRequest0(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMgclResponse *pRequest) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    (void)pRequest;
    return 0; // The binary leaves the result register as it found it.
}

int InitMGCLTask::OnServerRequest1(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMgclResponse *pRequest) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    rtMgclServerReply reply;
    memset(&reply, 0, sizeof(reply));
    strncpy(reply.messageId, pRequest->messageId, RT_MGCL_MESSAGE_ID_SIZE);
    reply.status = RT_MGCL_STATUS_SUCCESS;
    reply.sessionKey[0] = '\0';
    return rtMgclSendServerReply(&reply);
}

int InitMGCLTask::OnServerRequest2(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMgclResponse *pRequest) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    (void)pRequest;
    return 0; // The binary leaves the result register as it found it.
}

int InitMGCLTask::OnServerRequest3(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMgclResponse *pRequest) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    (void)pRequest;
    return 0; // The binary leaves the result register as it found it.
}
