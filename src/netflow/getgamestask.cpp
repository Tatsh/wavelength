#include "netflow/getgamestask.h"

#include <cstring>

#include "game/gamedb.h"
#include "netflow/lobbyrt.h"
#include "netflow/netlaunchpadinfo.h"

void GetGamesTask::OnStart() {
    TheLobbyRT->mLaunchpads.clear();
    rtMediusGameListRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.applicationId = LobbyRT::kApplicationId;
    request.pageId = static_cast<short>(TheLobbyRT->mLaunchpadsPage + 1);
    request.pageSize = static_cast<short>(TheLobbyRT->mLaunchpadsPageSize);
    const int nError = rtMediusGameList(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void GetGamesTask::OnResponse(int nConnection,
                              int nType,
                              int nSize,
                              const rtMediusGameListResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->header.messageId);
    GetGamesTask *pTask = pFound != nullptr ? dynamic_cast<GetGamesTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        NetLaunchpadInfo info;
        info.mLaunchpadId = pResponse->worldId;
        LobbyRT::DecodeGameName(&info.mParams,
                                &info.mPlayerCount,
                                &info.mConnectionType,
                                &info.mLaunchpadWorld,
                                pResponse->gameName);
        info.mOpen = TheGameDb->HasSong(info.mParams.mSong.c_str());
        info.mStatus = LobbyRT::LaunchpadStatusOf(pResponse->worldStatus);
        if (info.mPlayerCount >= info.mParams.mMaxPlayers) {
            info.mStatus = NetLaunchpadInfo::kStatusFull;
        }
        TheLobbyRT->mLaunchpads.push_back(info);
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
