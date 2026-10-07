#include "memcard/mcclosetask.h"

#include "memcard/memcard.h"

void MCCloseTask::OnClose(int nResult) {
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCCloseTask::OnStart() {
    MemcardClose(this, sFd);
}
