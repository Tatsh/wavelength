#include "netflow/dnastask.h"

#ifdef VIDEO_STANDARD_PAL

#include <cstdio>
#include <cstring>

#include "msg/dnasstatusmsg.h"
#include "msg/lobbyconnectresultmsg.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"
#include "os/system.h"

namespace {

// The arguments of sceDNAS2InitNoHDD().
constexpr int kLineType = 1;
constexpr int kThreadPriority = 48;
constexpr int kTimeoutSeconds = 120;
constexpr int kRetrySeconds = 52;
constexpr float kMsPerSecond = 1000.0f;

// The errors sceDNAS2Status_t::subCode reports with sceDNAS2_SS_SERVER_ERROR.
enum DNASError {
    kErrorCatalogThird = -703,
    kErrorCatalogSecond = -702,
    kErrorCatalogFirst = -701,
    kErrorHardwareMissing = -404,
    kErrorDiscUnreadable = -403,
    kErrorDiscWrong = -402,
    kErrorHardware = -401,
    kErrorTimeout = -105,
    kErrorServerNotInService = -104,
    kErrorTitleNotInServiceLast = -103,
    kErrorTitleNotInServiceFirst = -102,
    kErrorBusy = -101,
};

constexpr char kNetError[] = "dnas_net_error";
constexpr char kBusy[] = "dnas_busy";
constexpr char kTitleNotInService[] = "dnas_title_not_in_service";
constexpr char kServerNotInService[] = "dnas_server_not_in_service";
constexpr char kTimeout[] = "dnas_timeout";
constexpr char kSystemError[] = "dnas_sys_error";
constexpr char kHardwareError[] = "dnas_hardware_err";
constexpr char kDiscError[] = "dnas_disk_err";
constexpr char kCatalogError[] = "dnas_cat_err";

// The advice under an error.
constexpr char kHintCheckDates[] = "dnas_foot_check_dates";
constexpr char kHintCheckNet[] = "dnas_foot_check_net";
constexpr char kHintIfContact[] = "dnas_foot_if_contact";
constexpr char kHintContact[] = "dnas_foot_contact";
constexpr char kHintCentral[] = "dnas_foot_central";
constexpr char kHintCleanDisc[] = "dnas_foot_cleancd";
constexpr char kHintEmpty[] = "dnas_foot_empty";

void NotifyRunning(int nRunning) {
    DNASStatusMsg msg(nRunning);
    TheLobbyRT->mConnectTask.Notify(&msg);
}

} // namespace

int DNASTask::sStatusCode;
int DNASTask::sSubCode;

void DNASTask::OnStart() {
    mSkipped = !TheNetInet->IsDNASPending();
    if (mSkipped) {
        NotifyRunning(0);
        Finish(true, 0);
        return;
    }
    NotifyRunning(1);
    mStartMs = SystemMs();
    mLastCode = 0;
    mLastSubCode = 0;
    mLastProgress = 0;

    sceDNAS2TitleAuthData_t authData;
    authData.lineType = kLineType;
    // The original passes the title's DNAS credentials (an 8-byte pass phrase at 0x0042f950 and
    // 65536 bytes of authentication data at 0x0041d198), which are not included.
    authData.passPhrase.data = nullptr;
    authData.passPhrase.size = 0;
    authData.authData.data = nullptr;
    authData.authData.size = 0;
    sStatusCode = 0;
    sSubCode = 0;

    sceDNAS2TimeoutInfo_t timeout;
    std::memset(&timeout, 0, sizeof(timeout));
    timeout.timeout = kTimeoutSeconds;
    timeout.retry = kRetrySeconds;
    std::printf("sceDNAS2InitNoHDD():%d\n",
                sceDNAS2InitNoHDD(&authData, kThreadPriority, 0, 0, &timeout));

    const int nResult = sceDNAS2AuthNetStart();
    std::printf("sceDNAS2AuthNetStart():%d\n", nResult);
    if (nResult < 0) {
        Finish(false, LobbyConnectResultMsg::kResultDNASFailed);
    }
}

void DNASTask::OnStop() {
    if (mSkipped) {
        return;
    }
    std::printf("DNAS took %.1f seconds\n", (SystemMs() - mStartMs) / kMsPerSecond);
    std::printf("sceDNAS2Abort():%d\n", sceDNAS2Abort());
    NotifyRunning(0);
}

void DNASTask::OnPoll() {
    if (mSkipped) {
        return;
    }
    sceDNAS2Status_t status;
    std::memset(&status, 0, sizeof(status));
    const int nResult = sceDNAS2GetStatus(&status);
    if (nResult < 0) {
        std::printf("sceDNAS2GetStatus():%d\n", nResult);
        sStatusCode = sceDNAS2_SS_SERVER_ERROR;
        sSubCode = nResult;
        Finish(false, LobbyConnectResultMsg::kResultDNASFailed);
        return;
    }
    if (status.code != mLastCode || status.subCode != mLastSubCode ||
        status.progress != mLastProgress) {
        mLastCode = status.code;
        mLastSubCode = status.subCode;
        mLastProgress = status.progress;
        std::printf("DNASStatus code: %d, sub_code: %d, progress: %d\n",
                    status.code,
                    status.subCode,
                    status.progress);
    }
    sStatusCode = status.code;
    sSubCode = status.subCode;
    if (status.code == sceDNAS2_SS_COMPLETE) {
        Finish(true, 0);
        TheNetInet->ClearDNASPending();
    } else if (status.code == sceDNAS2_SS_SERVER_ERROR ||
               status.code == sceDNAS2_SS_NETWORK_ERROR) {
        Finish(false, LobbyConnectResultMsg::kResultDNASFailed);
    }
}

void DNASTask::GetError(int *pnSubCode, const char **ppszError, const char **ppszHint) {
    *pnSubCode = sSubCode;
    if (sStatusCode == sceDNAS2_SS_NETWORK_ERROR) {
        *ppszError = kNetError;
        *ppszHint = kHintCheckNet;
        return;
    }
    if (sStatusCode == sceDNAS2_SS_SERVER_ERROR) {
        switch (sSubCode) {
        case kErrorBusy:
            *ppszError = kBusy;
            *ppszHint = kHintEmpty;
            return;
        case kErrorTitleNotInServiceFirst:
        case kErrorTitleNotInServiceLast:
            *ppszError = kTitleNotInService;
            *ppszHint = kHintCheckDates;
            return;
        case kErrorServerNotInService:
            *ppszError = kServerNotInService;
            *ppszHint = kHintCentral;
            return;
        case kErrorTimeout:
            *ppszError = kTimeout;
            *ppszHint = kHintContact;
            return;
        case kErrorHardware:
        case kErrorHardwareMissing:
            *ppszError = kHardwareError;
            *ppszHint = kHintContact;
            return;
        case kErrorDiscWrong:
            *ppszError = kDiscError;
            *ppszHint = kHintContact;
            return;
        case kErrorDiscUnreadable:
            *ppszError = kDiscError;
            *ppszHint = kHintCleanDisc;
            return;
        case kErrorCatalogFirst:
        case kErrorCatalogSecond:
        case kErrorCatalogThird:
            *ppszError = kCatalogError;
            *ppszHint = kHintContact;
            return;
        default:
            break;
        }
    }
    *ppszError = kSystemError;
    *ppszHint = kHintIfContact;
}

#endif
