#include "memcard/mcgetremixdatatask.h"

#include "game/remixinfo.h"

MCGetRemixDataTask::MCGetRemixDataTask() {
    mOpen = new MCOpenCurrRemixTask;
    mSeek = new MCSeekTask;
    mRead = new MCReadTask;
    mClose = new MCCloseTask;
    Add(mOpen);
    Add(mSeek);
    Add(mRead);
    Add(mClose);
}

void MCGetRemixDataTask::Set(int nPort, void *pBuffer, int nSize) {
    mSize = nSize;
    mBuffer = pBuffer;
    mOpen->Set(nPort);
    mSeek->Set(RemixInfo::WireSize(), 0);
    mRead->Set(mBuffer, mSize);
}
