#include "memcard/mcmkdirtask.h"

#include "memcard/memcard.h"

void MCMkDirTask::Set(int nPort, const char *pszName) {
    SetPort(nPort);
    mName = pszName;
}

void MCMkDirTask::OnMkdir(int nResult) {
    SetStatus(nResult);
    if (sStatus == kStatusNotFound) {
        sStatus = kStatusOk;
    }
    Finish(sStatus == kStatusOk);
}

void MCMkDirTask::OnStart() {
    MemcardMkdir(this, mPort, mName.c_str());
}
