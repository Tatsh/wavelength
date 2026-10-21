#include "netflow/mgclauthenticationtask.h"

#include <cstring>

#include "msg/launchpadabortedmsg.h"
#include "netflow/hostpadrt.h"

void MGCLAuthenticationTask::OnStart() {
    rtMgclAuthenticationRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    request.requestType = 0;
    if (rtMgclAuthenticate(&request, OnResponse) != 0) {
        Finish(false, LaunchpadAbortedMsg::kReasonServer);
    }
}

void MGCLAuthenticationTask::OnResponse(int nConnection,
                                        int nType,
                                        int nSize,
                                        const rtMgclAuthenticationResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MGCL_STATUS_SUCCESS) {
        TheHostpadRT->SetServerInfo(pResponse->info);
    }
    pTask->Finish(pResponse->header.status == RT_MGCL_STATUS_SUCCESS,
                  LaunchpadAbortedMsg::kReasonServer);
}
