#include "memcard/mcreadtask.h"

#include "memcard/memcard.h"

void MCReadTask::Set(void *pBuffer, int nSize) {
    SetPort(mPort);
    mSize = nSize;
    mBuffer = pBuffer;
}

void MCReadTask::OnRead(int nResult) {
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCReadTask::OnStart() {
    MemcardRead(this, sFd, mBuffer, mSize);
}
