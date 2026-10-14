#include "netflow/waitforsharedonetask.h"

#include "netflow/hostpadrt.h"

void WaitForShareDoneTask::OnPoll() {
    if (TheHostpadRT != nullptr && TheHostpadRT->mSharing == 0) {
        Finish(true, 0);
    }
}
