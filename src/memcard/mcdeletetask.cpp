#include "memcard/mcdeletetask.h"

#include "memcard/memcard.h"

void MCDeleteTask::Set(int nPort, const char *pszName) {
    SetPort(nPort);
    mName = pszName;
}

void MCDeleteTask::OnDelete(int nResult) {
    SetStatus(nResult);
    if (sStatus == kStatusNotFound) {
        sStatus = kStatusOk;
    }
    Finish(sStatus == kStatusOk);
}

void MCDeleteTask::OnStart() {
    MemcardDelete(this, mPort, mName.c_str());
}
