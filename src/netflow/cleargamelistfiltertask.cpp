#include "netflow/cleargamelistfiltertask.h"

#include <cstring>

#include "msg/joinchatroomresultmsg.h"
#include "netflow/lobbyrt.h"

bool ClearGameListFilterTask::HasField(int nField, const std::vector<RTFilter> *pFilters) {
    for (const RTFilter &filter : *pFilters) {
        if (filter.mField == nField) {
            return true;
        }
    }
    return false;
}

void ClearGameListFilterTask::OnStart() {
    mPending = 0;
    for (const RTFilter &filter : TheLobbyRT->mActiveFilters) {
        if (HasField(filter.mField, &TheLobbyRT->mFilters)) {
            continue;
        }
        rtMediusIdRequest request;
        memset(&request, 0, sizeof(request));
        strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
        MakeTag(this, request.header.messageId);
        request.id = filter.mField;
        const int nError = rtMediusClearGameListFilter(&request, OnResponse);
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

void ClearGameListFilterTask::OnResponse(int nConnection,
                                         int nType,
                                         int nSize,
                                         const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    ClearGameListFilterTask *pTask =
        pFound != nullptr ? dynamic_cast<ClearGameListFilterTask *>(pFound) : nullptr;
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
