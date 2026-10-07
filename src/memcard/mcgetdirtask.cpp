#include "memcard/mcgetdirtask.h"

#include <libmc.h>

#include "memcard/memcard.h"

void MCGetDirTask::Set(
    MemcardSerialTask *pOwner, int nPort, const char *pszName, int nMaxEntries, int nMode) {
    SetPort(nPort);
    mOwner = pOwner;
    mName = pszName;
    mMode = nMode;
    mMaxEntries = nMaxEntries;
}

void MCGetDirTask::OnGetDir(int nResult, MemcardDirEntry *pEntries) {
    if (nResult == sceMcResNoEntry) {
        sDirCount = 0;
        sStatus = kStatusOk;
    } else {
        SetStatus(nResult);
        sDirEntries = pEntries;
        mMaxEntries = nResult;
        sDirCount = nResult;
    }
    Finish(sStatus == kStatusOk);
    if (mOwner != nullptr) {
        mOwner->OnDirListed();
    }
}

void MCGetDirTask::OnStart() {
    MemcardGetDir(this, mPort, mName.c_str(), mMaxEntries, mMode);
}
