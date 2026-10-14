#include "netflow/playerinfotask.h"

#include <cstring>

#include "msg/loginresultmsg.h"
#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"

void PlayerInfoTask::OnStart() {
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.id = mPlayer->mId;
    const int nError = rtMediusPlayerInfo(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultAccountNotFound);
    }
}

void PlayerInfoTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusPlayerInfoResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    PlayerInfoTask *pTask = pFound != nullptr ? dynamic_cast<PlayerInfoTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        pTask->mPlayer->mName = pResponse->accountName;
        pTask->mPlayer->mConnectionType = LobbyRT::ConnectionTypeOf(pResponse->connectionClass);
        LobbyRT::DecodePlayer(pTask->mPlayer, pResponse->stats);
    }
    pTask->Finish(pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS,
                  LoginResultMsg::kResultAccountNotFound);
}
