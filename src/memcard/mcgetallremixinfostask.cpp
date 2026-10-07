#include "memcard/mcgetallremixinfostask.h"

#include "memcard/memcardconfig.h"
#include "os/bufstream.h"

void MCGetAllRemixInfosTask::Set(int nPort, std::vector<RemixInfo> *pInfos) {
    SetPort(nPort);
    mInfos = pInfos;
    mIndex = 0;
    mIter = g_RemixNames.begin();
}

void MCGetAllRemixInfosTask::OnStart() {
    const int nCount = static_cast<int>(g_RemixNames.size());
    RemixInfo blank;
    mInfos->resize(nCount, blank);
    mIndex = 0;
    mIter = g_RemixNames.begin();
    ReadNext();
}

void MCGetAllRemixInfosTask::ReadNext() {
    if (mIndex < static_cast<int>(mInfos->size())) {
        mInfo.Set(mPort, mIter->c_str());
        mInfo.Start();
    } else {
        Finish(true);
    }
}

void MCGetAllRemixInfosTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mInfo.Poll();
    if (nState == kStateDone) {
        BufStream stream(mInfo.mBuffer, mInfo.mSize, true);
        stream >> mInfos->at(mIndex);
        ++mIndex;
        ++mIter;
        ReadNext();
    } else if (nState == kStateFailed) {
        Finish(false);
    }
}
