#include "netflow/getchannelstask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "netflow/netchatroominfo.h"
#include "os/utf8.h"

namespace {

// The page of rooms the request lists, and its size.
constexpr short kFirstPage = 1;
constexpr short kPageSize = 100;

} // namespace

void GetChannelsTask::OnStart() {
    TheLobbyRT->GetChatrooms().clear();
    rtMediusChannelListRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.pageId = kFirstPage;
    request.pageSize = kPageSize;
    const int nError = rtMediusChannelList(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1356, nError);
#else
    LobbyRT::ReportMediusError(1189, nError);
#endif
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
#ifdef VIDEO_STANDARD_PAL
        const String name = Utf8DecodedString(pResponse->name);
        room.mName = name.c_str();
#else
        room.mName = pResponse->name;
#endif
        room.mPlayerCount = pResponse->playerCount;
        // Yes, the binary gives every room the capacity of the room of the local player.
        room.mMaxPlayers = TheLobbyRT->GetChatroom().mMaxPlayers;
        TheLobbyRT->GetChatrooms().push_back(room);
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
