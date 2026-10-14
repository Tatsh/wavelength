#include "netflow/waitforwelcometask.h"

#include "netflow/launchpadrt.h"

void WaitForWelcomeTask::OnPoll() {
    if (TheLaunchpadRT == nullptr) {
        return;
    }
    if (TheLaunchpadRT->mState >= LaunchpadRT::kStateWelcomed) {
        Finish(true, 0);
    } else if (TheLaunchpadRT->mState == LaunchpadRT::kStateFailed) {
        Finish(false, TheLaunchpadRT->mError);
    }
}
