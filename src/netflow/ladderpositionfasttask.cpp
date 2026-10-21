#include "netflow/ladderpositionfasttask.h"

#include <cstring>

#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"

void LadderPositionFastTask::OnStart() {
    rtMediusLadderPositionFastRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.accountId = mPlayer->mId;
    request.statValue = mPlayer->mRating;
    const int nError = rtMediusLadderPositionFast(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1868, nError);
#else
    LobbyRT::ReportMediusError(1675, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
    }
}

void LadderPositionFastTask::OnResponse(int nConnection,
                                        int nType,
                                        int nSize,
                                        const rtMediusLadderPositionResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    LadderPositionFastTask *pTask =
        pFound != nullptr ? dynamic_cast<LadderPositionFastTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS) {
        pTask->mPlayer->mRank = pResponse->rank;
    }
    pTask->Finish(true, 0);
}
