#include "memcard/mccardstatustask.h"

MCCardStatusTask::MCCardStatusTask() {
    mGetInfo = new MCGetInfoTask;
    Add(mGetInfo);
}

void MCCardStatusTask::Set(int nPort) {
    mGetInfo->Set(nullptr, nPort, 0);
}
