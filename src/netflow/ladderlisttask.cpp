#include "netflow/ladderlisttask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "netflow/playerinfotask.h"
#include "os/system.h"
#include "os/utf8.h"
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
    }
}

void LadderListTask::OnStart() {
    TheLobbyRT->GetRankedPlayers().clear();
    rtMediusLadderListRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.ladderType = kLadderType;
    request.statId = kLadderStat;
    request.reserved30 = 0;
    request.startPosition = TheLobbyRT->GetRanksPageSize() * TheLobbyRT->GetRanksPage() + 1;
    request.pageSize = TheLobbyRT->GetRanksPageSize();
    const int nError = rtMediusLadderList(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1644, nError);
#else
    LobbyRT::ReportMediusError(1456, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
    }
}

void LadderListTask::StartNextInfo() {
    if (mNext == TheLobbyRT->GetRankedPlayers().end()) {
        Finish(true, 0);
        return;
    }
    mInfoTask = std::make_unique<PlayerInfoTask>(&*mNext, 0);
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
    mInfoTask.reset();
    int nRankNewbies = 0;
    SystemConfig()->FindArray("net", false)->FindBool("rank_newbies", &nRankNewbies, false);
    if (nRankNewbies == 0 && mNext->IsNewbie()) {
        TheLobbyRT->GetRankedPlayers().erase(mNext, TheLobbyRT->GetRankedPlayers().end());
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
#ifdef VIDEO_STANDARD_PAL
        const String name = Utf8DecodedString(pResponse->accountName);
        player.mName = name.c_str();
#else
        player.mName = pResponse->accountName;
#endif
        player.mRank = pResponse->rank;
        player.mRating = kRankedRating;
        TheLobbyRT->GetRankedPlayers().push_back(player);
        if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    if (TheLobbyRT->GetRanksMode() != 0) {
        pTask->Finish(true, 0);
        return;
    }
    pTask->mNext = TheLobbyRT->GetRankedPlayers().begin();
    pTask->StartNextInfo();
}
