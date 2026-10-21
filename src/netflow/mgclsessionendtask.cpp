#include "netflow/mgclsessionendtask.h"

#include <cstring>

#include <librtmedius.h>

#include "netflow/hostpadrt.h"
#include "netflow/lobbyrt.h"
#include "os/system.h"

namespace {

// The wait for the answer of the server, in milliseconds.
constexpr float kAnswerWaitMs = 3000.0f;

} // namespace

void MGCLSessionEndTask::OnStart() {
    if (TheHostpadRT->GetSessionKey() == "") {
        Finish(true, 0);
        return;
    }
    mDeadline = SystemMs() + kAnswerWaitMs;
    if (TheHostpadRT->GetWorldId() != LaunchpadRT::kNoWorld) {
        rtMediusEndGameReportData report;
        memset(&report, 0, sizeof(report));
        strcpy(report.sessionKey, TheLobbyRT->GetSessionKey().c_str());
        report.worldId = TheHostpadRT->GetWorldId();
        report.winningTeam[0] = '\0';
        report.winningPlayer[0] = '\0';
        report.finalScore = 0;
        (void)rtMediusEndGameReport(&report);
        TheHostpadRT->SetWorldId(LaunchpadRT::kNoWorld);
    }

    rtMgclSessionEndRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    if (rtMgclSessionEnd(&request, OnResponse) != 0 || TheLobbyRT->GetDisconnected() != 0) {
        TheHostpadRT->SetSessionKey("");
        Finish(true, 0);
    }
}

void MGCLSessionEndTask::OnPoll() {
    if (mDeadline < SystemMs()) {
        Finish(true, 0);
    }
}

void MGCLSessionEndTask::OnResponse(int nConnection,
                                    int nType,
                                    int nSize,
                                    const rtMgclResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask != nullptr) {
        TheHostpadRT->SetSessionKey("");
        pTask->Finish(true, 0);
    }
}
