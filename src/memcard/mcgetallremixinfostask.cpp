#include "memcard/mcgetallremixinfostask.h"

#include "memcard/memcardconfig.h"
#include "os/bufstream.h"
#ifdef VIDEO_STANDARD_PAL
#include "os/memcardsync.h"
#endif

void MCGetAllRemixInfosTask::Set(int nPort, std::vector<RemixInfo> *pInfos) {
    SetPort(nPort);
    mInfos = pInfos;
    mIndex = 0;
#ifndef VIDEO_STANDARD_PAL
    mIter = g_RemixNames.begin();
#endif
}

void MCGetAllRemixInfosTask::OnStart() {
#ifdef VIDEO_STANDARD_PAL
    mInfos->clear();
    mInfos->reserve(g_RemixNames.size());
    mIndex = 0;
#else
    const int nCount = static_cast<int>(g_RemixNames.size());
    RemixInfo blank;
    mInfos->resize(nCount, blank);
    mIndex = 0;
    mIter = g_RemixNames.begin();
#endif
    ReadNext();
}

void MCGetAllRemixInfosTask::ReadNext() {
#ifdef VIDEO_STANDARD_PAL
    if (static_cast<unsigned int>(mIndex) < g_RemixNames.size()) {
        mInfo.Set(mPort, g_RemixNames[mIndex].c_str());
#else
    if (mIndex < static_cast<int>(mInfos->size())) {
        mInfo.Set(mPort, mIter->c_str());
#endif
        mInfo.Start();
    } else {
        Finish(true);
    }
}

void MCGetAllRemixInfosTask::OnPoll() {
    MemcardTask::OnPoll();
    const int nState = mInfo.Poll();
    if (nState == kStateDone) {
#ifdef VIDEO_STANDARD_PAL
        // A file of another size than a whole description is deleted as corrupt.
        if (MemcardReadSizeIs(RemixInfo::WireSize())) {
            BufStream stream(mInfo.mBuffer, mInfo.mSize, true);
            mInfos->push_back(RemixInfo());
            stream >> mInfos->back();
        } else {
            MemcardDeleteAndWait(mPort, g_MemcardOpenPath.c_str());
        }
        ++mIndex;
#else
        BufStream stream(mInfo.mBuffer, mInfo.mSize, true);
        stream >> mInfos->at(mIndex);
        ++mIndex;
        ++mIter;
#endif
        ReadNext();
    } else if (nState == kStateFailed) {
        Finish(false);
    }
}
