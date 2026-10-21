#include "netflow/accountupdatestatstask.h"

#include <cstring>

#include "msg/loginresultmsg.h"
#include "netflow/lobbyrt.h"

void AccountUpdateStatsTask::OnStart() {
    rtMediusUpdateStatsRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    strncpy(request.stats, TheLobbyRT->GetPlayerStats(), RT_MEDIUS_STATS_SIZE);
    const int nError = rtMediusUpdateStats(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(967, nError);
#else
    LobbyRT::ReportMediusError(838, nError);
#endif
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultStatsFailed);
    }
}

void AccountUpdateStatsTask::OnResponse(int nConnection,
                                        int nType,
                                        int nSize,
                                        const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask != nullptr) {
        pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS,
                      LoginResultMsg::kResultStatsFailed);
    }
}
