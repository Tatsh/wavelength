#include "netflow/anonymouslogintask.h"

#include <cstring>

#include <librtmedius.h>

#include "msg/loginresultmsg.h"
#include "netflow/accountlogintask.h"
#include "netflow/lobbyrt.h"

void AnonymousLoginTask::OnStart() {
    rtMediusAnonymousLoginRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    strncpy(request.accountName, TheLobbyRT->GetName(), RT_MEDIUS_PLAYER_NAME_SIZE);
    strncpy(request.stats, TheLobbyRT->GetPlayerStats(), RT_MEDIUS_STATS_SIZE);
    const int nError = rtMediusAnonymousLogin(&request, AccountLoginTask::OnResponse);
    LobbyRT::ReportMediusError(742, nError);
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultServerError);
    }
}
