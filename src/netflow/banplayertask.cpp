#include "netflow/banplayertask.h"

#include <cstring>

#include "netflow/lobbyrt.h"

void BanPlayerTask::OnStart() {
    rtMediusBanPlayerRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    request.worldId = mWorldId;
    request.accountId = mAccountId;
    request.duration = kBanDuration;
    request.reserved34 = 0;
    (void)rtMediusBanPlayer(&request, OnResponse);
    Finish(true, 0);
}

void BanPlayerTask::OnResponse(int nConnection,
                               int nType,
                               int nSize,
                               const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    (void)pResponse;
}
