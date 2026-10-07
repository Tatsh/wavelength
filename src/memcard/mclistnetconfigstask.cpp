#include "memcard/mclistnetconfigstask.h"

#include "memcard/memcardtask.h"

MCListNetConfigsTask::MCListNetConfigsTask() {
    mGetInfo = new MCGetInfoTask;
    mNetConfig = new MCNetConfigTask;
    Add(mGetInfo);
    Add(mNetConfig);
}

void MCListNetConfigsTask::Set(int nPort) {
    mGetInfo->Set(this, nPort, 0);
    mNetConfig->Stop();
    mNetConfig->Set(nPort);
}

void MCListNetConfigsTask::OnCardInfo() {
    if (MemcardTask::sStatus != MemcardTask::kStatusOk) {
        return;
    }
    if (mGetInfo->mFormat == 0) {
        MemcardTask::sStatus = MemcardTask::kStatusUnformatted;
        mGetInfo->Finish(false);
    }
}

std::list<InetConfig> *MCListNetConfigsTask::GetConfigs() {
    return &mNetConfig->mConfigs;
}
