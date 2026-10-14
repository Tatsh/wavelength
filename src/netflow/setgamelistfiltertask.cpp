#include "netflow/setgamelistfiltertask.h"

#include <algorithm>
#include <cstring>

#include "msg/joinchatroomresultmsg.h"
#include "netflow/lobbyrt.h"
#include "netflow/rtfilter.h"

void SetGameListFilterTask::OnStart() {
    mPending = 0;
    for (const RTFilter &filter : TheLobbyRT->mFilters) {
        if (std::find(TheLobbyRT->mActiveFilters.begin(),
                      TheLobbyRT->mActiveFilters.end(),
                      filter) != TheLobbyRT->mActiveFilters.end()) {
            continue;
        }
        rtMediusGameListFilterRequest request;
        memset(&request, 0, sizeof(request));
        strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
        MakeTag(this, request.header.messageId);
        request.filterField = filter.mField;
        request.comparison = filter.mComparison;
        request.value = filter.mValue;
        const int nError = rtMediusSetGameListFilter(&request, OnResponse);
        LobbyRT::ReportMediusError(__LINE__, nError);
        if (nError != 0) {
            Finish(false, JoinChatroomResultMsg::kResultFailed);
            return;
        }
        ++mPending;
    }
    if (mPending == 0) {
        Finish(true, 0);
    }
}

void SetGameListFilterTask::OnResponse(int nConnection,
                                       int nType,
                                       int nSize,
                                       const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    SetGameListFilterTask *pTask =
        pFound != nullptr ? dynamic_cast<SetGameListFilterTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    --pTask->mPending;
    if (pResponse->status != RT_MEDIUS_STATUS_SUCCESS) {
        pTask->Finish(false, JoinChatroomResultMsg::kResultFailed);
    } else if (pTask->mPending == 0) {
        pTask->Finish(true, 0);
    }
}
