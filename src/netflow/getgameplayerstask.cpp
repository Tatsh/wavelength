#include "netflow/getgameplayerstask.h"

#include <cstring>

#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"
#include "os/utf8.h"

void GetGamePlayersTask::OnStart() {
    TheLobbyRT->mLaunchpadPlayers.clear();
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.id = TheLobbyRT->mLaunchpadId;
    const int nError = rtMediusGamePlayers(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1559, nError);
#else
    LobbyRT::ReportMediusError(1376, nError);
#endif
    if (nError != 0) {
        Finish(true, 0);
    }
}

void GetGamePlayersTask::OnResponse(int nConnection,
                                    int nType,
                                    int nSize,
                                    const rtMediusGamePlayersResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    GetGamePlayersTask *pTask =
        pFound != nullptr ? dynamic_cast<GetGamePlayersTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        LobbyPlayer player;
        player.mId = pResponse->accountId;
#ifdef VIDEO_STANDARD_PAL
        const String name = Utf8DecodedString(pResponse->accountName);
        player.mName = name.c_str();
#else
        player.mName = pResponse->accountName;
#endif
        player.mConnectionType = LobbyRT::ConnectionTypeOf(pResponse->connectionClass);
        player.mRank = 0;
        LobbyRT::DecodePlayer(&player, pResponse->stats);
        TheLobbyRT->mLaunchpadPlayers.push_back(player);
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
