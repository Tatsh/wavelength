#include "netflow/sessionendtask.h"

#include <cstring>

#include "netflow/lobbyrt.h"
#include "os/system.h"

namespace {

// The longest wait for the response, in milliseconds.
constexpr float kResponseWaitMs = 3000.0f;

} // namespace

void SessionEndTask::OnStart() {
    mDeadline = SystemMs() + kResponseWaitMs;
    if (TheLobbyRT->GetSessionKey() == "") {
        Finish(true, 0);
        return;
    }
    rtMediusRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId); // Yes, the binary sends the request without the key.
    const int nError = rtMediusSessionEnd(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(463, nError);
#else
    LobbyRT::ReportMediusError(447, nError);
#endif
    if (nError != 0 || TheLobbyRT->GetDisconnected() != 0) {
        TheLobbyRT->SetSessionKey("");
        Finish(true, 0);
    }
}

void SessionEndTask::OnPoll() {
    if (mDeadline < SystemMs()) {
        Finish(true, 0);
    }
}

void SessionEndTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask == nullptr) {
        return;
    }
    TheLobbyRT->SetSessionKey("");
    pTask->Finish(true, 0);
}
