#include "memcard/mcreadtask.h"

#include "memcard/memcard.h"
#include "memcard/memcardconfig.h"

void MCReadTask::Set(void *pBuffer, int nSize) {
    SetPort(mPort);
    mSize = nSize;
    mBuffer = pBuffer;
}

void MCReadTask::OnRead(int nResult) {
#ifdef VIDEO_STANDARD_PAL
    g_nMemcardReadResult = nResult;
#endif
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCReadTask::OnStart() {
    MemcardRead(this, sFd, mBuffer, mSize);
}
