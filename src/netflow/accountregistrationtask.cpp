#include "netflow/accountregistrationtask.h"

#include <cstring>
#include <strings.h>

#include "msg/createaccountresultmsg.h"
#include "msg/loginresultmsg.h"
#include "netflow/lobbyrt.h"
#include "os/system.h"
#include "os/utf8.h"
#include "script/dataarray.h"

namespace {

// The prefix of the names of the accounts of the developer, which the game does not create.
const char *const kReservedPrefix = "HMX";
constexpr int kReservedPrefixLength = 3;

// The kind of account the game creates.
constexpr int kAccountTypeMaster = 1;

// The statuses of the server.
constexpr int kStatusAccountExists = -999;
constexpr int kStatusVulgarName = -967;

} // namespace

void AccountRegistrationTask::OnStart() {
#ifdef VIDEO_STANDARD_PAL
    int nAllowReserved = 0;
    SystemConfig()->FindArray("net", false)->FindBool("allow_hmx", &nAllowReserved, false);
    if (nAllowReserved == 0 &&
        strncasecmp(TheLobbyRT->GetName(), kReservedPrefix, kReservedPrefixLength) == 0) {
#else
    if (strncasecmp(TheLobbyRT->GetName(), kReservedPrefix, kReservedPrefixLength) == 0) {
#endif
        Finish(false, CreateAccountResultMsg::kResultAlreadyExists);
        return;
    }
    rtMediusAccountRegistrationRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->GetSessionKey().c_str());
    MakeTag(this, request.header.messageId);
    request.accountType = kAccountTypeMaster;
#ifdef VIDEO_STANDARD_PAL
    strncpy(
        request.accountName, Utf8String(TheLobbyRT->GetName()).c_str(), RT_MEDIUS_PLAYER_NAME_SIZE);
    strncpy(
        request.password, Utf8String(TheLobbyRT->GetPassword()).c_str(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusAccountRegistration(&request, OnResponse);
    LobbyRT::ReportMediusError(708, nError);
#else
    strncpy(request.accountName, TheLobbyRT->GetName(), RT_MEDIUS_PLAYER_NAME_SIZE);
    strncpy(request.password, TheLobbyRT->GetPassword(), RT_MEDIUS_PASSWORD_SIZE);
    const int nError = rtMediusAccountRegistration(&request, OnResponse);
    LobbyRT::ReportMediusError(666, nError);
#endif
    if (nError != 0) {
        Finish(false, LoginResultMsg::kResultServerError);
    }
}

void AccountRegistrationTask::OnResponse(int nConnection,
                                         int nType,
                                         int nSize,
                                         const rtMediusResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->messageId);
    if (pTask == nullptr) {
        return;
    }
    bool bSucceeded = false;
    int nResult;
    switch (pResponse->status) {
    case kStatusVulgarName:
        nResult = CreateAccountResultMsg::kResultVulgarName;
        break;
    case kStatusAccountExists:
        nResult = CreateAccountResultMsg::kResultAlreadyExists;
        break;
    case RT_MEDIUS_STATUS_SUCCESS:
        bSucceeded = true;
        nResult = 0;
        break;
    default:
        nResult = CreateAccountResultMsg::kResultRegistrationFailed;
        break;
    }
    pTask->Finish(bSucceeded, nResult);
}
