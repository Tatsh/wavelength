#include "netflow/accountupdatepasswordtask.h"

#include <cstring>

#include "msg/changepasswordresultmsg.h"
#include "netflow/lobbyrt.h"
#include "os/utf8.h"

void AccountUpdatePasswordTask::OnStart() {
    const char *pszNewPassword = TheLobbyRT->mNewPassword.c_str();
    rtMediusChangePasswordRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
#ifdef VIDEO_STANDARD_PAL
    strncpy(request.oldPassword,
            Utf8String(TheLobbyRT->GetPassword()).c_str(),
            RT_MEDIUS_PASSWORD_SIZE);
    strncpy(request.newPassword, Utf8String(pszNewPassword).c_str(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusChangePassword(&request, OnResponse);
    LobbyRT::ReportMediusError(1013, nError);
#else
    strncpy(request.oldPassword, TheLobbyRT->GetPassword(), RT_MEDIUS_PASSWORD_SIZE);
    strncpy(request.newPassword, pszNewPassword, RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusChangePassword(&request, OnResponse);
    LobbyRT::ReportMediusError(872, nError);
#endif
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
