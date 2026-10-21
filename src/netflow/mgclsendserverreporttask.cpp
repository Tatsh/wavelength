#include "netflow/mgclsendserverreporttask.h"

#include <cstring>

#include <librtmgcl.h>

#include "netflow/hostpadrt.h"

namespace {

// The games and players the host reports.
constexpr short kReportWorlds = 1;
constexpr short kReportPlayers = 1;

} // namespace

void MGCLSendServerReportTask::OnStart() {
    rtMgclServerReport report;
    memset(&report, 0, sizeof(report));
    strcpy(report.sessionKey, TheHostpadRT->GetSessionKey().c_str());
    report.maxPlayersPerWorld = LaunchpadRT::kMaxClients;
    report.activePlayers = kReportPlayers;
    report.maxWorlds = kReportWorlds;
    report.activeWorlds = kReportWorlds;
    report.alertLevel = 0;
    (void)rtMgclSendServerReport(&report);
    Finish(true, 0);
}
