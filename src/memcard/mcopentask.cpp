#include "memcard/mcopentask.h"

#include "memcard/memcard.h"

void MCOpenTask::Set(int nPort, const char *pszName, int nMode) {
    SetPort(nPort);
    mName = pszName;
    mMode = nMode;
}

void MCOpenTask::OnOpen(int nResult) {
    sFd = nResult;
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCOpenTask::OnStart() {
    MemcardOpen(this, mPort, mName.c_str(), mMode);
}
