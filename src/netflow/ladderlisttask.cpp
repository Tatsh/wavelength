#include "netflow/ladderlisttask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "netflow/playerinfotask.h"
#include "os/system.h"
#include "script/dataarray.h"

namespace {

// The ladder and the statistic the ranking reads.
constexpr int kLadderType = 2;
constexpr int kLadderStat = 11;

// The rating of a player of the ranking before its details are read, which is not new.
constexpr int kRankedRating = 1000000;

} // namespace

LadderListTask::~LadderListTask() {
    if (mInfoTask != nullptr) {
        mInfoTask->Stop();
        delete mInfoTask;
    }
}

void LadderListTask::OnStart() {
    TheLobbyRT->mRankedPlayers.clear();
    rtMediusLadderListRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.ladderType = kLadderType;
    request.statId = kLadderStat;
    request.reserved30 = 0;
    request.startPosition = TheLobbyRT->mRanksPageSize * TheLobbyRT->mRanksPage + 1;
    request.pageSize = TheLobbyRT->mRanksPageSize;
    const int nError = rtMediusLadderList(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void LadderListTask::StartNextInfo() {
    if (mNext == TheLobbyRT->mRankedPlayers.end()) {
        Finish(true, 0);
        return;
    }
    mInfoTask = new PlayerInfoTask(&*mNext, 0);
    mInfoTask->Start();
}

void LadderListTask::OnPoll() {
    if (mInfoTask == nullptr) {
        return;
    }
    const int nState = mInfoTask->Poll();
    if (nState != kStateDone && nState != kStateFailed) {
        return;
    }
    mInfoTask->Stop();
    delete mInfoTask;
    mInfoTask = nullptr;
    int nRankNewbies = 0;
    SystemConfig()->FindArray("net", false)->FindBool("rank_newbies", &nRankNewbies, false);
    if (nRankNewbies == 0 && mNext->IsNewbie()) {
        TheLobbyRT->mRankedPlayers.erase(mNext, TheLobbyRT->mRankedPlayers.end());
        Finish(true, 0);
        return;
    }
    ++mNext;
    StartNextInfo();
}

void LadderListTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusLadderListResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    LadderListTask *pTask = pFound != nullptr ? dynamic_cast<LadderListTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS) {
        LobbyPlayer player;
        player.mId = pResponse->accountId;
        player.mName = pResponse->accountName;
        player.mRank = pResponse->rank;
        player.mRating = kRankedRating;
        TheLobbyRT->mRankedPlayers.push_back(player);
        if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    if (TheLobbyRT->mRanksMode != 0) {
        pTask->Finish(true, 0);
        return;
    }
    pTask->mNext = TheLobbyRT->mRankedPlayers.begin();
    pTask->StartNextInfo();
}
