#include "memcard/mcgetremixinfotask.h"

#include <libmc.h>

#include "game/remixinfo.h"

MCGetRemixInfoTask::MCGetRemixInfoTask() {
    mSize = RemixInfo::WireSize();
    mBuffer = new char[mSize];
    mOpen = new MCOpenTask;
    mRead = new MCReadTask;
    mClose = new MCCloseTask;
    Add(mOpen);
    Add(mRead);
    Add(mClose);
}

MCGetRemixInfoTask::~MCGetRemixInfoTask() {
    delete[] mBuffer;
}

void MCGetRemixInfoTask::Set(int nPort, const char *pszName) {
    (void)GetState(); // Yes, the binary discards this call's result.
    mOpen->Set(nPort, pszName, sceMcFileAttrReadable);
    mRead->Set(mBuffer, mSize);
}
