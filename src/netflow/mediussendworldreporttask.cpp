#include "netflow/mediussendworldreporttask.h"

#include <cstring>

#include <librtmedius.h>

#include "netflow/hostpadrt.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"
#include "os/string.h"

namespace {

// The player count the report gives, whatever the players of the game.
constexpr int kReportPlayerCount = 2;

// The players a game takes at least.
constexpr int kMinPlayers = 1;

} // namespace

void MediusSendWorldReportTask::OnStart() {
    const int nPlayers = static_cast<int>(TheHostpadRT->GetPlayers()->size());
    const NetGameParams *pParams = &TheHostpadRT->GetParams();
    String name;
    LobbyRT::EncodeGameName(&name,
                            pParams,
                            nPlayers,
                            TheNetInet->GetConnectionType(),
                            TheLobbyRT->GetLocalPlayer().GetId());
    int nRules;
    int nRulesVariant;
    LobbyRT::EncodeRules(&nRules, &nRulesVariant, pParams->GetRuleSet(), pParams->GetSkillLevel());
    int nLevel;
    LobbyRT::EncodeSong(&nLevel, pParams->GetSong().c_str());

    rtMediusWorldReportData report;
    memset(&report, 0, sizeof(report));
    strcpy(report.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    report.playerCount = kReportPlayerCount;
    report.worldId = TheLaunchpadRT->GetWorldId();
    strncpy(report.gameName, name.c_str(), RT_MEDIUS_GAME_NAME_SIZE);
    report.minPlayers = kMinPlayers;
    report.maxPlayers = LaunchpadRT::kMaxClients;
    report.gameLevel = nLevel;
    report.rulesSet = nRules;
    report.gameStats[0] = '\0';
    report.playerSkillLevel = 0;
    report.genericField1 = 0;
    report.genericField2 = TheLobbyRT->GetChatroom().mId;
    report.genericField3 = TheHostpadRT->CanJoin();
    report.worldStatus = TheHostpadRT->GetWorldStatus();
    (void)rtMediusWorldReport(&report);
    Finish(true, 0);
}
