#include "netflow/killlaunchpadtask.h"

#include "msg/launchpadabortedmsg.h"
#include "netflow/launchpadrt.h"

void KillLaunchpadTask::OnStart() {
    if (TheLaunchpadRT != nullptr) {
        TheLaunchpadRT->Abort(LaunchpadAbortedMsg::kReasonLost);
    }
    Finish(true, 0);
}
