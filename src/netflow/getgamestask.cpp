#include "netflow/getgamestask.h"

#include <cstring>

#include "game/gamedb.h"
#include "netflow/lobbyrt.h"
#include "netflow/netlaunchpadinfo.h"
#include "os/utf8.h"

void GetGamesTask::OnStart() {
    TheLobbyRT->GetLaunchpads().clear();
    rtMediusGameListRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.applicationId = LobbyRT::ApplicationId();
    request.pageId = static_cast<short>(TheLobbyRT->GetLaunchpadsPage() + 1);
    request.pageSize = static_cast<short>(TheLobbyRT->GetLaunchpadsPageSize());
    const int nError = rtMediusGameList(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(1424, nError);
#else
    LobbyRT::ReportMediusError(1252, nError);
#endif
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
#ifdef VIDEO_STANDARD_PAL
        const String gameName = Utf8DecodedString(pResponse->gameName);
        LobbyRT::DecodeGameName(&info.mParams,
                                &info.mPlayerCount,
                                &info.mConnectionType,
                                &info.mLaunchpadWorld,
                                gameName.c_str());
#else
        LobbyRT::DecodeGameName(&info.mParams,
                                &info.mPlayerCount,
                                &info.mConnectionType,
                                &info.mLaunchpadWorld,
                                pResponse->gameName);
#endif
        info.mOpen = TheGameDb->HasSong(info.mParams.GetSong().c_str());
        info.mStatus = LobbyRT::LaunchpadStatusOf(pResponse->worldStatus);
        if (info.mPlayerCount >= info.mParams.GetMaxPlayers()) {
            info.mStatus = NetLaunchpadInfo::kStatusFull;
        }
        TheLobbyRT->GetLaunchpads().push_back(info);
        if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
