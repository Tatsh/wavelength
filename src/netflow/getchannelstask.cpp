#include "netflow/getchannelstask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "netflow/netchatroominfo.h"

namespace {

// The page of rooms the request lists, and its size.
constexpr short kFirstPage = 1;
constexpr short kPageSize = 100;

} // namespace

void GetChannelsTask::OnStart() {
    TheLobbyRT->mChatrooms.clear();
    rtMediusChannelListRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.pageId = kFirstPage;
    request.pageSize = kPageSize;
    const int nError = rtMediusChannelList(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void GetChannelsTask::OnResponse(int nConnection,
                                 int nType,
                                 int nSize,
                                 const rtMediusChannelListResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    GetChannelsTask *pTask = pFound != nullptr ? dynamic_cast<GetChannelsTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        NetChatroomInfo room;
        room.mId = pResponse->worldId;
        room.mName = pResponse->name;
        room.mPlayerCount = pResponse->playerCount;
        // Yes, the binary gives every room the capacity of the room of the local player.
        room.mMaxPlayers = TheLobbyRT->mChatroom.mMaxPlayers;
        TheLobbyRT->mChatrooms.push_back(room);
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
