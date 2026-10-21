#include "netflow/ladderpositiontask.h"

#include <cstring>

#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"

namespace {

// The ladder and the statistic the ranking reads.
constexpr int kLadderType = 2;
constexpr int kLadderStat = 11;

} // namespace

void LadderPositionTask::OnStart() {
    rtMediusLadderPositionRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.accountId = mPlayer->GetId();
    request.ladderType = kLadderType;
    request.statId = kLadderStat;
    request.reserved34 = 0;
    const int nError = rtMediusLadderPosition(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1769, nError);
#else
    LobbyRT::ReportMediusError(1576, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
    }
}

void LadderPositionTask::OnResponse(int nConnection,
                                    int nType,
                                    int nSize,
                                    const rtMediusLadderPositionResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    LadderPositionTask *pTask =
        pFound != nullptr ? dynamic_cast<LadderPositionTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    const int nRank = pResponse->status == RT_MEDIUS_STATUS_SUCCESS ? pResponse->rank : 0;
    pTask->mPlayer->SetRank(nRank);
    pTask->Finish(true, 0);
}
