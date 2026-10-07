#include "memcard/mcdeletesavedremixtask.h"

#include "memcard/memcardconfig.h"

void MCDeleteSavedRemixTask::Set(int nPort) {
    SetPort(nPort);
}

void MCDeleteSavedRemixTask::OnStart() {
    mDelete.Set(mPort, g_RemixPath.c_str());
    mDelete.Start();
}

void MCDeleteSavedRemixTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mDelete.Poll();
    if (nState == kStateDone) {
        Finish(true);
    } else if (nState == kStateFailed) {
        Finish(false);
    }
}
