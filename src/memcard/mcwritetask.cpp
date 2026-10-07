#include "memcard/mcwritetask.h"

#include "memcard/memcard.h"

void MCWriteTask::Set(const void *pData, int nSize) {
    SetPort(mPort);
    mSize = nSize;
    mData = pData;
}

void MCWriteTask::OnWrite(int nResult) {
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCWriteTask::OnStart() {
    MemcardWrite(this, sFd, mData, mSize);
}
