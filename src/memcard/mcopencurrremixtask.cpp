#include "memcard/mcopencurrremixtask.h"

#include <libmc.h>

#include "memcard/memcardconfig.h"

void MCOpenCurrRemixTask::Set(int nPort) {
    mPort = nPort;
}

void MCOpenCurrRemixTask::OnStart() {
    mOpen.Set(mPort, g_RemixPath.c_str(), sceMcFileAttrReadable);
    mOpen.Start();
}

void MCOpenCurrRemixTask::OnPoll() {
    MemcardTask::OnPoll();
    // Yes, the binary fails the task when the open has not finished on its first poll.
    if (mOpen.Poll() == kStateDone) {
        Finish(true);
    } else {
        Finish(false);
    }
}
