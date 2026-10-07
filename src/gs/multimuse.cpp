#include "gs/multimuse.h"

#include <algorithm>

namespace {

bool TickBefore(const MultiMuse::TimedMuse &left, const MultiMuse::TimedMuse &right) {
    return left.mTick < right.mTick;
}

} // namespace

void MultiMuse::MultiMuseCmd::Execute() {
    mMulti->Advance();
}

MultiMuse::MultiMuse(int nStopPrevious)
    : mMuses(), mNext(mMuses.end()), mFirstActive(mMuses.end()), mStopPrevious(nStopPrevious),
      mScheduler(nullptr), mCmd(new MultiMuseCmd(this)), mLength(0), mOffset(0), mEnd(0) {
}

MultiMuse::~MultiMuse() {
    Stop();
}

void MultiMuse::Play(Scheduler *pScheduler) {
    PlayFrom(pScheduler, 0);
}

void MultiMuse::PlayFrom(Scheduler *pScheduler, int nOffset) {
    PlayWindow(pScheduler, nOffset, mLength + 1);
}

void MultiMuse::PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) {
    Stop();
    if (!(nStart < nEnd)) {
        return;
    }
    mScheduler = pScheduler;
    mNext = mFirstActive = mMuses.begin();
    while (mNext != mMuses.end() && mNext->mTick <= nStart && mNext->mTick < nEnd) {
        mNext->mMuse->PlayWindow(pScheduler, nStart - mNext->mTick, nEnd - mNext->mTick);
        ++mNext;
    }
    if (mNext == mMuses.end() || !(mNext->mTick < nEnd)) {
        mScheduler = nullptr;
        mNext = mFirstActive = mMuses.end();
        return;
    }
    mOffset = mScheduler->mTick - nStart;
    mEnd = mOffset + nEnd;
    mScheduler->PostAt(mCmd.Get(), mOffset + mNext->mTick, false);
}

bool MultiMuse::IsPlaying() {
    if (mScheduler != nullptr) {
        return true;
    }
    for (auto it = mFirstActive; it != mNext; ++it) {
        if (it->mMuse->IsPlaying()) {
            return true;
        }
    }
    return false;
}

int MultiMuse::GetLength() {
    return mLength;
}

void MultiMuse::Advance() {
    if (mStopPrevious) {
        while (mFirstActive != mNext) {
            mFirstActive->mMuse->Stop();
            ++mFirstActive;
        }
    }
    const int nTick = mNext->mTick;
    while (mNext != mMuses.end() && mNext->mTick == nTick) {
        mNext->mMuse->PlayWindow(mScheduler, 0, mEnd - mOffset - nTick);
        ++mNext;
    }
    if (mNext != mMuses.end() && mNext->mTick + mOffset < mEnd) {
        mScheduler->PostAt(mCmd.Get(), mOffset + mNext->mTick, false);
    } else {
        mScheduler = nullptr;
        mFirstActive = mNext = mMuses.end();
    }
}

void MultiMuse::Stop() {
    if (mScheduler != nullptr) {
        mScheduler->Cancel(mCmd.Get());
    }
    mScheduler = nullptr;
    mFirstActive = mNext = mMuses.end();
    for (TimedMuse &timed : mMuses) {
        timed.mMuse->Stop();
    }
}

Muse *MultiMuse::Clone() {
    MultiMuse *pCopy = new MultiMuse(0);
    for (TimedMuse &timed : mMuses) {
        pCopy->Add(timed.mMuse.Get()->Clone(), timed.mTick);
    }
    return pCopy;
}

void MultiMuse::SetNoteCB(NoteCB *pNoteCB) {
    for (TimedMuse &timed : mMuses) {
        timed.mMuse->SetNoteCB(pNoteCB);
    }
}

void MultiMuse::Add(Muse *pMuse, int nTick) {
    const TimedMuse timed{Ptr<Muse>(pMuse), nTick};
    std::list<TimedMuse>::iterator it;
    if (mMuses.empty() || !(nTick < mMuses.back().mTick)) {
        it = mMuses.insert(mMuses.end(), timed);
    } else {
        it =
            mMuses.insert(std::upper_bound(mMuses.begin(), mMuses.end(), timed, TickBefore), timed);
    }
    if (mScheduler != nullptr && std::prev(mNext) == it && !(nTick < mScheduler->mTick - mOffset) &&
        mOffset + nTick < mEnd) {
        // The new muse starts before the one the command waits on.
        mScheduler->Cancel(mCmd.Get());
        mScheduler->PostAt(mCmd.Get(), mOffset + nTick, false);
        mNext = it;
    }
    const int nEnd = nTick + pMuse->GetLength();
    if (mLength < nEnd) {
        mLength = nEnd;
    }
}
