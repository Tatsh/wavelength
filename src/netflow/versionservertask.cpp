#include "netflow/versionservertask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "os/debug.h"

void VersionServerTask::OnStart() {
    rtMediusRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.messageId);
    const int nError = rtMediusVersionServer(&request, OnResponse);
    LobbyRT::ReportMediusError(494, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void VersionServerTask::OnResponse(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMediusVersionResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask == nullptr) {
        return;
    }
    DebugPrint("Server Version:%s\n", pResponse->version);
    pTask->Finish(true, 0);
}
