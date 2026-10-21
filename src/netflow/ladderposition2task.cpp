#include "netflow/ladderposition2task.h"

#include <cstring>

#include "app/msgsink.h"
#include "msg/playerrankmsg.h"
#include "netflow/lobbyrt.h"

namespace {

// The ladder and the statistic the ranking reads.
constexpr int kLadderType = 2;
constexpr int kLadderStat = 11;

} // namespace

void LadderPosition2Task::OnStart() {
    rtMediusLadderPositionRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.accountId = mAccount;
    request.ladderType = kLadderType;
    request.statId = kLadderStat;
    request.reserved34 = 0;
    const int nError = rtMediusLadderPosition(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1818, nError);
#else
    LobbyRT::ReportMediusError(1625, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
        if (mSink != nullptr) {
            PlayerRankMsg msg(mAccount, 0);
            mSink->Dispatch(&msg);
        }
    }
}

void LadderPosition2Task::OnResponse(int nConnection,
                                     int nType,
                                     int nSize,
                                     const rtMediusLadderPositionResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    LadderPosition2Task *pTask =
        pFound != nullptr ? dynamic_cast<LadderPosition2Task *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    const int nRank = pResponse->status == RT_MEDIUS_STATUS_SUCCESS ? pResponse->rank : 0;
    pTask->Finish(true, 0);
    if (pTask->mSink != nullptr) {
        PlayerRankMsg msg(pTask->mAccount, nRank);
        pTask->mSink->Dispatch(&msg);
    }
}
