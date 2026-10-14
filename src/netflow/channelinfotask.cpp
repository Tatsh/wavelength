#include "netflow/channelinfotask.h"

#include <cstring>

#include "msg/joinchatroomresultmsg.h"
#include "netflow/lobbyrt.h"
#include "netflow/netchatroominfo.h"

namespace {

// The identifier of no room.
constexpr int kNoChatroom = -1;

} // namespace

void ChannelInfoTask::OnStart() {
    if (mChatroom->mId == kNoChatroom) {
        mChatroom->mName.Clear();
        mChatroom->mPlayerCount = 0;
        mChatroom->mMaxPlayers = 0;
        Finish(true, 0);
        return;
    }
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.id = mChatroom->mId;
    const int nError = rtMediusChannelInfo(&request, OnResponse);
    LobbyRT::ReportMediusError(984, nError);
    if (nError != 0) {
        Finish(false, JoinChatroomResultMsg::kResultFailed);
    }
}

void ChannelInfoTask::OnResponse(int nConnection,
                                 int nType,
                                 int nSize,
                                 const rtMediusChannelInfoResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    ChannelInfoTask *pTask = pFound != nullptr ? dynamic_cast<ChannelInfoTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        pTask->mChatroom->mName = pResponse->name;
        pTask->mChatroom->mPlayerCount = pResponse->playerCount;
        pTask->mChatroom->mMaxPlayers = pResponse->maxPlayers;
    }
    pTask->Finish(pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS,
                  JoinChatroomResultMsg::kResultFailed);
}
