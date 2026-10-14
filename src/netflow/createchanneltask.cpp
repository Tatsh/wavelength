#include "netflow/createchanneltask.h"

#include <cstring>

#include "msg/joinchatroomresultmsg.h"
#include "netflow/lobbyrt.h"

namespace {

// The statuses of the server.
constexpr int kStatusNameExists = -974;
constexpr int kStatusVulgarName = -967;

} // namespace

void CreateChannelTask::OnStart() {
    rtMediusCreateChannelRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.applicationId = LobbyRT::kApplicationId;
    request.maxPlayers = TheLobbyRT->mChatroom.mMaxPlayers;
    request.password[0] = '\0';
    strncpy(request.name, TheLobbyRT->mNewChatroomName.c_str(), RT_MEDIUS_GAME_NAME_SIZE);
    const int nError = rtMediusCreateChannel(&request, OnResponse);
    LobbyRT::ReportMediusError(1831, nError);
    if (nError != 0) {
        Finish(false, JoinChatroomResultMsg::kResultFailed);
    }
}

void CreateChannelTask::OnResponse(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMediusCreateChannelResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    const int nStatus = pResponse->header.status;
    if (nStatus == RT_MEDIUS_STATUS_SUCCESS) {
        TheLobbyRT->mJoinChatroomId = pResponse->worldId;
        pTask->Finish(true, 0);
    } else if (nStatus == kStatusNameExists) {
        pTask->Finish(false, JoinChatroomResultMsg::kResultNameExists);
    } else if (nStatus == kStatusVulgarName || pResponse->worldId == kStatusVulgarName) {
        // Yes, the binary also tests the room for the status.
        pTask->Finish(false, JoinChatroomResultMsg::kResultVulgarName);
    } else {
        pTask->Finish(false, JoinChatroomResultMsg::kResultFailed);
    }
}
