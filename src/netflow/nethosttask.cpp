#include "netflow/nethosttask.h"

#include <cstring>

#include <librtdme.h>

#include "msg/launchpadabortedmsg.h"
#include "netflow/launchpadrt.h"

void NetHostTask::OnStart() {
    rtDmeHostParams params;
    memset(&params, 0, sizeof(params));
    (void)rtDmeInitHostParams(&params);
    params.onClientLeave = LaunchpadRT::OnNetClientLeave;
    params.onConnect = LaunchpadRT::OnNetConnect;
    params.onClientJoin = LaunchpadRT::OnNetClientJoin;
    params.maxClients = LaunchpadRT::kMaxClients;
    rtDmeConnectStatus status;
    memset(&status, 0, sizeof(status));
    LaunchpadRT::sNetTask = this;
    if (rtDmeHost(&params, &status) != 0) {
        LaunchpadRT::sNetTask->Finish(false, LaunchpadAbortedMsg::kReasonHostFailed);
    }
}

void NetHostTask::OnStop() {
    LaunchpadRT::sNetTask = nullptr;
}
