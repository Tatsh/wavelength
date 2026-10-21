#include "netflow/mgclsessionbegintask.h"

#include <cstring>

#include "msg/launchpadabortedmsg.h"
#include "netflow/hostpadrt.h"
#include "netflow/lobbyrt.h"

namespace {

// The kind of server the host registers as.
constexpr int kServerType = 2;

} // namespace

void MGCLSessionBeginTask::OnStart() {
    rtMgclSessionBeginRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    request.applicationId = LobbyRT::ApplicationId();
    request.locationId = TheLobbyRT->GetLocationId();
    request.serverType = kServerType;
    if (rtMgclSessionBegin(&request, OnResponse) != 0) {
        Finish(false, LaunchpadAbortedMsg::kReasonServer);
    }
}

void MGCLSessionBeginTask::OnResponse(int nConnection,
                                      int nType,
                                      int nSize,
                                      const rtMgclSessionBeginResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MGCL_STATUS_SUCCESS) {
        TheHostpadRT->SetSessionKey(pResponse->sessionKey);
    } else {
        TheHostpadRT->SetSessionKey("");
    }
    pTask->Finish(pResponse->header.status == RT_MGCL_STATUS_SUCCESS,
                  LaunchpadAbortedMsg::kReasonServer);
}
