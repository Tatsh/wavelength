#include "netflow/getlobbyplayerstask.h"

#include <cstring>

#include "netflow/lobbyplayer.h"
#include "netflow/lobbyrt.h"

void GetLobbyPlayersTask::OnStart() {
    TheLobbyRT->mLobbyPlayers.clear();
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.id = TheLobbyRT->mChatroom.mId;
    const int nError = rtMediusChannelPlayers(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void GetLobbyPlayersTask::OnResponse(int nConnection,
                                     int nType,
                                     int nSize,
                                     const rtMediusChannelPlayersResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    GetLobbyPlayersTask *pTask =
        pFound != nullptr ? dynamic_cast<GetLobbyPlayersTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        LobbyPlayer player;
        player.mId = pResponse->accountId;
        player.mName = pResponse->accountName;
        player.mConnectionType = LobbyRT::ConnectionTypeOf(pResponse->connectionClass);
        player.mRank = 0;
        LobbyRT::DecodePlayer(&player, pResponse->stats);
        TheLobbyRT->mLobbyPlayers.push_back(player);
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
