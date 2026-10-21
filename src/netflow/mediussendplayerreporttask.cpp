#include "netflow/mediussendplayerreporttask.h"

#include <cstring>

#include <librtmedius.h>

#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"

void MediusSendPlayerReportTask::OnStart() {
    rtMediusPlayerReportData report;
    memset(&report, 0, sizeof(report));
    strcpy(report.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    report.worldId = TheLaunchpadRT->GetWorldId();
    strncpy(report.stats, TheLobbyRT->GetPlayerStats(), RT_MEDIUS_STATS_SIZE);
    (void)rtMediusPlayerReport(&report);
    Finish(true, 0);
}
