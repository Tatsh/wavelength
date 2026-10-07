#include "gs/muselooper.h"

void MuseLooper::RepeatCmd::Execute() {
    mLooper->mMuse->Stop();
    mLooper->mMuse->Play(mLooper->mScheduler);
    mLooper->mScheduler->PostIn(this, mLooper->mLength, false);
}

MuseLooper::MuseLooper(Muse *pMuse, int nLength)
    : mRepeatCmd(new RepeatCmd(this)), mMuse(pMuse), mLength(nLength), mStartTick(0),
      mScheduler(nullptr) {
}

MuseLooper::~MuseLooper() {
    Stop();
}

void MuseLooper::Play(Scheduler *pScheduler, int nPosition) {
    Stop();
    const int nInLoop = nPosition % mLength;
    mMuse->PlayFrom(pScheduler, nInLoop);
    pScheduler->PostIn(mRepeatCmd.Get(), mLength - nInLoop, false);
    mScheduler = pScheduler;
    mStartTick = pScheduler->mTick - nInLoop;
}

void MuseLooper::Stop() {
    if (mScheduler == nullptr) {
        return;
    }
    mScheduler->Cancel(mRepeatCmd.Get());
    mMuse->Stop();
    mScheduler = nullptr;
}

bool MuseLooper::IsPlaying() const {
    return mScheduler != nullptr;
}

int MuseLooper::GetLength() const {
    return mLength;
}

int MuseLooper::GetPosition() const {
    if (mScheduler == nullptr) {
        return 0;
    }
    return (mScheduler->mTick - mStartTick) % mLength;
}
