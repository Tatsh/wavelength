#include "memcard/mcdeleteremixtask.h"

#include "memcard/memcardconfig.h"

MCDeleteRemixTask::MCDeleteRemixTask() {
    mGetInfo = new MCGetInfoTask;
    mFind = new MCFindRemixTask;
    mDelete = new MCDeleteSavedRemixTask;
    Add(mGetInfo);
    Add(mFind);
    Add(mDelete);
}

void MCDeleteRemixTask::Set(int nPort, const char *pszName) {
    mGetInfo->Set(nullptr, nPort, 0);
    mFind->Set(nPort, FormatString("%s%s", pszName, g_pszRemixExt));
    mDelete->Set(nPort);
}
