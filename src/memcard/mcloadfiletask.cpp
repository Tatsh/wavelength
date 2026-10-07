#include "memcard/mcloadfiletask.h"

#include <libmc.h>

MCLoadFileTask::MCLoadFileTask() : mOwner(nullptr) {
    mOpen = new MCOpenTask;
    mRead = new MCReadTask;
    mClose = new MCCloseTask;
    Add(mOpen);
    Add(mRead);
    Add(mClose);
}

void MCLoadFileTask::Set(
    MemcardSerialTask *pOwner, int nPort, const char *pszName, void *pBuffer, int nSize) {
    (void)GetState(); // Yes, the binary discards this call's result.
    mOwner = pOwner;
    mOpen->Set(nPort, pszName, sceMcFileAttrReadable);
    mRead->Set(pBuffer, nSize);
}

void MCLoadFileTask::OnStop() {
    if (mOwner != nullptr) {
        mOwner->OnFileLoaded();
    }
}
