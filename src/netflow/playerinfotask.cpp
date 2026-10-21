#include "netflow/playerinfotask.h"

#include <cstring>

#include "msg/loginresultmsg.h"
#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"
#include "os/utf8.h"

void PlayerInfoTask::OnStart() {
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.id = mPlayer->GetId();
    const int nError = rtMediusPlayerInfo(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1079, nError);
#else
    LobbyRT::ReportMediusError(932, nError);
#endif
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
#ifdef VIDEO_STANDARD_PAL
        const String name = Utf8DecodedString(pResponse->accountName);
        pTask->mPlayer->SetName(name.c_str());
#else
        pTask->mPlayer->SetName(pResponse->accountName);
#endif
        pTask->mPlayer->SetConnectionType(LobbyRT::ConnectionTypeOf(pResponse->connectionClass));
        LobbyRT::DecodePlayer(pTask->mPlayer, pResponse->stats);
    }
    pTask->Finish(pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS,
                  LoginResultMsg::kResultAccountNotFound);
}
