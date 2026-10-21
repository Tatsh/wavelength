#include "netflow/anonymouslogintask.h"

#include <cstring>

#include <librtmedius.h>

#include "msg/loginresultmsg.h"
#include "netflow/accountlogintask.h"
#include "netflow/lobbyrt.h"
#include "os/utf8.h"

void AnonymousLoginTask::OnStart() {
    rtMediusAnonymousLoginRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
#ifdef VIDEO_STANDARD_PAL
    strncpy(
        request.accountName, Utf8String(TheLobbyRT->GetName()).c_str(), RT_MEDIUS_PLAYER_NAME_SIZE);
#else
    strncpy(request.accountName, TheLobbyRT->GetName(), RT_MEDIUS_PLAYER_NAME_SIZE);
#endif
    strncpy(request.stats, TheLobbyRT->GetPlayerStats(), RT_MEDIUS_STATS_SIZE);
    const int nError = rtMediusAnonymousLogin(&request, AccountLoginTask::OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(796, nError);
#else
    LobbyRT::ReportMediusError(742, nError);
#endif
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultServerError);
    }
}
