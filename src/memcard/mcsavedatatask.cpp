#include "memcard/mcsavedatatask.h"

MCSaveDataTask::MCSaveDataTask() {
    mGetInfo = new MCGetInfoTask;
    mSaveFile = new MCSaveFileTask;
    Add(mGetInfo);
    Add(mSaveFile);
}

void MCSaveDataTask::Set(int nPort, const char *pszName, const void *pData, int nSize) {
    (void)GetState(); // Yes, the binary discards this call's result.
    mGetInfo->Set(this, nPort, 1);
    mSaveFile->Set(nPort, pszName, pData, nSize);
}
