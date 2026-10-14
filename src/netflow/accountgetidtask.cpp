#include "netflow/accountgetidtask.h"

#include <cstring>

#include "msg/loginresultmsg.h"
#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"

void AccountGetIDTask::OnStart() {
    rtMediusAccountGetIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    strncpy(request.accountName, mPlayer->mName.c_str(), RT_MEDIUS_PLAYER_NAME_SIZE);
    const int nError = rtMediusAccountGetId(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultAccountNotFound);
    }
}

void AccountGetIDTask::OnResponse(int nConnection,
                                  int nType,
                                  int nSize,
                                  const rtMediusAccountGetIdResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    AccountGetIDTask *pTask =
        pFound != nullptr ? dynamic_cast<AccountGetIDTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    pTask->mPlayer->mId = pResponse->accountId;
    pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS,
                  LoginResultMsg::kResultAccountNotFound);
}
