#include "netflow/waitforwelcometask.h"

#include "netflow/launchpadrt.h"

void WaitForWelcomeTask::OnPoll() {
    if (TheLaunchpadRT == nullptr) {
        return;
    }
    if (TheLaunchpadRT->GetState() >= LaunchpadRT::kStateWelcomed) {
        Finish(true, 0);
    } else if (TheLaunchpadRT->GetState() == LaunchpadRT::kStateFailed) {
        Finish(false, TheLaunchpadRT->GetError());
    }
}
