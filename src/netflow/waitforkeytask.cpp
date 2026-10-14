#include "netflow/waitforkeytask.h"

#include "netflow/joinpadrt.h"

void WaitForKeyTask::OnPoll() {
    if (TheJoinpadRT != nullptr && TheJoinpadRT->mKeyReceived != 0) {
        Finish(true, 0);
    }
}
