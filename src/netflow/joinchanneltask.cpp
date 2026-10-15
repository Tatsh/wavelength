#include "netflow/joinchanneltask.h"

#include <cstring>

#include "msg/joinchatroomresultmsg.h"
#include "netflow/lobbyrt.h"

void JoinChannelTask::OnStart() {
    rtMediusJoinChannelRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.worldId = TheLobbyRT->mJoinChatroomId;
    request.password[0] = '\0';
    const int nError = rtMediusJoinChannel(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1982, nError);
#else
    LobbyRT::ReportMediusError(1789, nError);
#endif
    if (nError != 0) {
        Finish(false, JoinChatroomResultMsg::kResultFailed);
    }
}

void JoinChannelTask::OnResponse(int nConnection,
                                 int nType,
                                 int nSize,
                                 const rtMediusJoinChannelResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        TheLobbyRT->SetAccount(
            TheLobbyRT->mLocalPlayer.mId, TheLobbyRT->mJoinChatroomId, &pResponse->info);
    }
    pTask->Finish(pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS,
                  JoinChatroomResultMsg::kResultFailed);
}
