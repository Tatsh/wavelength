#include "netflow/netconnecttask.h"

#include <cstring>

#include <librtdme.h>

#include "msg/launchpadabortedmsg.h"
#include "netflow/launchpadrt.h"
#include "os/system.h"

namespace {

// The wait for the connection, in milliseconds.
constexpr float kConnectWaitMs = 15000.0f;

} // namespace

void NetConnectTask::OnStart() {
    mDeadline = SystemMs() + kConnectWaitMs;
    rtDmeConnectParams params;
    memset(&params, 0, sizeof(params));
    (void)rtDmeInitConnectParams(&params);
    params.maxClients = LaunchpadRT::kMaxClients;
    params.info = TheLaunchpadRT->mConnectionInfo;
    params.onClientLeave = LaunchpadRT::OnNetClientLeave;
    params.onConnect = LaunchpadRT::OnNetConnect;
    params.onClientJoin = LaunchpadRT::OnNetClientJoin;
    params.onClientReady = nullptr;
    rtDmeConnectStatus status;
    memset(&status, 0, sizeof(status));
    LaunchpadRT::sNetTask = this;
    if (rtDmeConnect(&params, &status) != 0) {
        Finish(false, LaunchpadAbortedMsg::kReasonConnectFailed);
    }
}

void NetConnectTask::OnPoll() {
    if (mDeadline < SystemMs()) {
        Finish(false, LaunchpadAbortedMsg::kReasonIncompatibleHost);
    }
}

void NetConnectTask::OnStop() {
    LaunchpadRT::sNetTask = nullptr;
}
