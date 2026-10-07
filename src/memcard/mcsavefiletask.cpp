#include "memcard/mcsavefiletask.h"

#include <libmc.h>

MCSaveFileTask::MCSaveFileTask() {
    mOpen = new MCOpenTask;
    mWrite = new MCWriteTask;
    mClose = new MCCloseTask;
    Add(mOpen);
    Add(mWrite);
    Add(mClose);
}

void MCSaveFileTask::Set(int nPort, const char *pszName, const void *pData, int nSize) {
    (void)GetState(); // Yes, the binary discards this call's result.
    mOpen->Set(nPort, pszName, sceMcFileCreateFile | sceMcFileAttrWriteable);
    mWrite->Set(pData, nSize);
}
