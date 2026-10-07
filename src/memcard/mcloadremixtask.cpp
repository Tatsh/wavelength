#include "memcard/mcloadremixtask.h"

#include "memcard/memcardconfig.h"
#include "os/string.h"

MCLoadRemixTask::MCLoadRemixTask() {
    mGetInfo = new MCGetInfoTask;
    mFind = new MCFindRemixTask;
    mData = new MCGetRemixDataTask;
    Add(mGetInfo);
    Add(mFind);
    Add(mData);
}

void MCLoadRemixTask::Set(int nPort, const char *pszName, void *pBuffer, int nSize) {
    mGetInfo->Set(nullptr, nPort, 0);
    String fileName;
    fileName.Printf("%s%s", pszName, g_pszRemixExt);
    mFind->Set(nPort, fileName.c_str());
    mData->Set(nPort, pBuffer, nSize);
}
