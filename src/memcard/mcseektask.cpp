#include "memcard/mcseektask.h"

#include "memcard/memcard.h"

void MCSeekTask::Set(int nOffset, int nMode) {
    mMode = nMode;
    mOffset = nOffset;
}

void MCSeekTask::OnSeek(int nResult) {
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCSeekTask::OnStart() {
    MemcardSeek(this, sFd, mOffset, mMode);
}
