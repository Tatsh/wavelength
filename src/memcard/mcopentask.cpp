#include "memcard/mcopentask.h"

#include "memcard/memcard.h"
#include "memcard/memcardconfig.h"

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
#ifdef VIDEO_STANDARD_PAL
    g_MemcardOpenPath = mName;
#endif
    MemcardOpen(this, mPort, mName.c_str(), mMode);
}
