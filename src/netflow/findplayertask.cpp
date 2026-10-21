#include "netflow/findplayertask.h"

#include <cstring>

#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"
#include "netflow/netchatroominfo.h"

namespace {

// The identifier of no room.
constexpr int kNoChatroom = -1;

// The search by account.
constexpr int kSearchByAccount = 0;

// The kind of location of a player in a chat room.
constexpr int kLocationChannel = 1;

} // namespace

void FindPlayerTask::OnStart() {
    mChatroom->mId = kNoChatroom;
    if (mPlayer->mConnectionType == LobbyPlayer::kConnectionOffline) {
        Finish(true, 0);
        return;
    }
    rtMediusFindPlayerRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.searchType = kSearchByAccount;
    request.accountId = mPlayer->mId;
    const int nError = rtMediusFindPlayer(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1923, nError);
#else
    LobbyRT::ReportMediusError(1730, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
    }
}

void FindPlayerTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusFindPlayerResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    FindPlayerTask *pTask = pFound != nullptr ? dynamic_cast<FindPlayerTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        if (pResponse->online == 0) {
            pTask->mChatroom->mId = kNoChatroom; // Yes, the binary overwrites the value next.
        }
        if (pResponse->locationType == kLocationChannel) {
            pTask->mChatroom->mId = pResponse->worldId;
        } else {
            pTask->mChatroom->mId = pTask->mPlayer->mChannelId;
        }
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
