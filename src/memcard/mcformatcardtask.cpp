#include "memcard/mcformatcardtask.h"

MCFormatCardTask::MCFormatCardTask() {
    mGetInfo = new MCGetInfoTask;
    mFormat = new MCFormatTask;
    Add(mGetInfo);
    Add(mFormat);
}

void MCFormatCardTask::Set(int nPort) {
    mFormat->Stop();
    mGetInfo->Set(this, nPort, 0);
    mFormat->Set(nPort);
}

void MCFormatCardTask::OnCardInfo() {
    if (mGetInfo->mFormat != 0) {
        MemcardTask::sStatus = MemcardTask::kStatusFormatted;
        mGetInfo->Finish(false);
    }
}
