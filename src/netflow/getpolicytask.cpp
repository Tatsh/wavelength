#include "netflow/getpolicytask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "os/utf8.h"

namespace {

// The policy the request reads.
constexpr int kUsagePolicy = 0;

} // namespace

void GetPolicyTask::OnStart() {
    TheLobbyRT->SetPolicy("");
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.id = kUsagePolicy;
    const int nError = rtMediusGetPolicy(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1295, nError);
#else
    LobbyRT::ReportMediusError(1133, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
    }
}

void GetPolicyTask::OnResponse(int nConnection,
                               int nType,
                               int nSize,
                               const rtMediusPolicyResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
#ifdef VIDEO_STANDARD_PAL
        const String policy = Utf8DecodedString(pResponse->policy);
        TheLobbyRT->GetPolicy() << policy.c_str();
#else
        TheLobbyRT->GetPolicy() << pResponse->policy;
#endif
        // Yes, the binary tests the status again.
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfText == 0 &&
            pResponse->policy[0] != '\0') {
            return;
        }
    }
    pTask->Finish(true, 0);
}
