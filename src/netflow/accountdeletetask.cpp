#include "netflow/accountdeletetask.h"

#include <cstring>

#include "msg/deleteaccountresultmsg.h"
#include "msg/loginresultmsg.h"
#include "netflow/lobbyrt.h"
#include "os/utf8.h"

void AccountDeleteTask::OnStart() {
    rtMediusAccountDeleteRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
#ifdef VIDEO_STANDARD_PAL
    strncpy(
        request.password, Utf8String(TheLobbyRT->GetPassword()).c_str(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusAccountDelete(&request, OnResponse);
    LobbyRT::ReportMediusError(758, nError);
#else
    strncpy(request.password, TheLobbyRT->GetPassword(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusAccountDelete(&request, OnResponse);
    LobbyRT::ReportMediusError(710, nError);
#endif
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
