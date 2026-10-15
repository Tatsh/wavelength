#include "netflow/mgclcreategametask.h"

#include <cstring>

#include "msg/launchpadabortedmsg.h"
#include "netflow/hostpadrt.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"
#include "os/string.h"

namespace {

// The players a game takes at least.
constexpr int kMinClients = 1;

// The state of the game the host creates.
constexpr int kWorldStatusOpen = 1;

} // namespace

// NTSC-U/C: 0x003b0ae4, PAL: 0x0042f9b0
MGCLCreateGameTask *MGCLCreateGameTask::sTask;

void MGCLCreateGameTask::OnStart() {
    String name;
    sTask = this;
    const NetGameParams *pParams = &TheHostpadRT->mParams;
    LobbyRT::EncodeGameName(
        &name, pParams, 1, TheNetInet->GetConnectionType(), TheLobbyRT->mLocalPlayer.mId);
    int nRules;
    int nRulesVariant;
    LobbyRT::EncodeRules(&nRules, &nRulesVariant, pParams->mRuleSet, pParams->mSkillLevel);
    int nLevel;
    LobbyRT::EncodeSong(&nLevel, pParams->mSong.c_str());

    rtMgclCreateGameRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.gameName, name.c_str(), RT_MGCL_GAME_NAME_SIZE);
    request.applicationId = LobbyRT::ApplicationId();
    request.gameLevel = nLevel;
    request.rulesSet = nRules;
    request.maxClients = LaunchpadRT::kMaxClients;
    request.gameStats[0] = '\0';
    request.password[0] = '\0';
    request.minClients = kMinClients;
    request.playerSkillLevel = 0;
    request.genericField1 = 0;
    request.genericField2 = TheLobbyRT->mChatroom.mId;
    request.genericField3 = TheHostpadRT->CanJoin();
    request.gameHostType = LaunchpadRT::kGameHostPeerToPeer;
    request.worldStatus = kWorldStatusOpen;
    if (rtMgclCreateGame(&request, OnResponse) != 0) {
        sTask = nullptr;
        Finish(false, LaunchpadAbortedMsg::kReasonInit);
    }
}

void MGCLCreateGameTask::OnResponse(int nConnection,
                                    int nType,
                                    int nSize,
                                    const rtMgclCreateGameResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    if (sTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MGCL_STATUS_SUCCESS) {
        TheHostpadRT->mWorldId = pResponse->worldId;
    }
    sTask->Finish(pResponse->header.status == RT_MGCL_STATUS_SUCCESS,
                  LaunchpadAbortedMsg::kReasonCreateFailed);
}
