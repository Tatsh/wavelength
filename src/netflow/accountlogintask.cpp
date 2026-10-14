#include "netflow/accountlogintask.h"

#include <cstring>

#include "msg/loginresultmsg.h"
#include "netflow/lobbyrt.h"

namespace {

// The statuses of the server.
constexpr int kStatusAccountNotFound = -998;
constexpr int kStatusAlreadyLoggedIn = -997;
constexpr int kStatusInvalidPassword = -978;

} // namespace

void AccountLoginTask::OnStart() {
    rtMediusAccountLoginRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    strncpy(request.accountName, TheLobbyRT->GetName(), RT_MEDIUS_PLAYER_NAME_SIZE);
    strncpy(request.password, TheLobbyRT->GetPassword(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusAccountLogin(&request, OnResponse);
    LobbyRT::ReportMediusError(764, nError);
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultServerError);
    }
}

void AccountLoginTask::OnResponse(int nConnection,
                                  int nType,
                                  int nSize,
                                  const rtMediusLoginResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    TheLobbyRT->SetAccount(pResponse->accountId, pResponse->worldId, &pResponse->info);
    bool bSucceeded = false;
    int nResult = LoginResultMsg::kResultServerError;
    switch (pResponse->header.status) {
    case kStatusAlreadyLoggedIn:
        nResult = LoginResultMsg::kResultAlreadyLoggedIn;
        break;
    case kStatusAccountNotFound:
        nResult = LoginResultMsg::kResultAccountNotFound;
        break;
    case kStatusInvalidPassword:
        nResult = LoginResultMsg::kResultInvalidPassword;
        break;
    case RT_MEDIUS_STATUS_SUCCESS:
        bSucceeded = true;
        break;
    default:
        break;
    }
    pTask->Finish(bSucceeded, nResult);
}
