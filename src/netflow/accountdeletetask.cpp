#include "netflow/accountdeletetask.h"

#include <cstring>

#include "msg/deleteaccountresultmsg.h"
#include "msg/loginresultmsg.h"
#include "netflow/lobbyrt.h"

void AccountDeleteTask::OnStart() {
    rtMediusAccountDeleteRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    strncpy(request.password, TheLobbyRT->GetPassword(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusAccountDelete(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultServerError);
    }
}

void AccountDeleteTask::OnResponse(int nConnection,
                                   int nType,
                                   int nSize,
                                   const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask != nullptr) {
        pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS,
                      DeleteAccountResultMsg::kResultDeleteFailed);
    }
}
