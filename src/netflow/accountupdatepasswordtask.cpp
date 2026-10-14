#include "netflow/accountupdatepasswordtask.h"

#include <cstring>

#include "msg/changepasswordresultmsg.h"
#include "netflow/lobbyrt.h"

void AccountUpdatePasswordTask::OnStart() {
    const char *pszNewPassword = TheLobbyRT->mNewPassword.c_str();
    rtMediusChangePasswordRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    strncpy(request.oldPassword, TheLobbyRT->GetPassword(), RT_MEDIUS_PASSWORD_SIZE);
    strncpy(request.newPassword, pszNewPassword, RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusChangePassword(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, ChangePasswordResultMsg::kResultUpdateFailed);
    }
}

void AccountUpdatePasswordTask::OnResponse(int nConnection,
                                           int nType,
                                           int nSize,
                                           const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask != nullptr) {
        pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS,
                      ChangePasswordResultMsg::kResultUpdateFailed);
    }
}
