#include "memcard/mclistremixestask.h"

MCListRemixesTask::MCListRemixesTask() {
    mGetInfo = new MCGetInfoTask;
    mNames = new MCGetRemixNamesTask;
    mInfos = new MCGetAllRemixInfosTask;
    Add(mGetInfo);
    Add(mNames);
    Add(mInfos);
}

void MCListRemixesTask::Set(int nPort, std::vector<RemixInfo> *pInfos) {
    mGetInfo->Set(nullptr, nPort, 0);
    mNames->Set(nPort);
    mInfos->Set(nPort, pInfos);
}
