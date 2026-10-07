#include "game/gemcatcher.h"

#include <cmath>
#include <cstdlib>

#include "game/gameconfig.h"
#include "game/stats.h"
#include "os/scheduler.h"

GemCatcher::CatchCheckCmd::CatchCheckCmd(Receiver *pReceiver,
                                         CatchTrackState *pState,
                                         const float *pMsPerTick,
                                         int nTrack,
                                         int nLane,
                                         int nSlopMs,
                                         int nTicksPerBar)
    : mReceiver(pReceiver), mState(pState), mMsPerTick(pMsPerTick), mTrack(nTrack), mLane(nLane),
      mSlopMs(nSlopMs), mTicksPerBar(nTicksPerBar), mId(TheDefaultCommandId) {
}

void GemCatcher::CatchCheckCmd::Schedule() {
    mCursor = mState->GetCursor(mLane, TheSongScheduler.mTick);
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAtTime(this,
                                    static_cast<float>(mCursor.GetTick()) * *mMsPerTick +
                                        static_cast<float>(mSlopMs),
                                    mId,
                                    false);
    }
}

void GemCatcher::CatchCheckCmd::Execute() {
    if (mCursor.IsValid()) {
        const int nBar = mCursor.GetTick() / mTicksPerBar;
        if (!mState->IsCaptured(nBar) && mState->IsEnabled(nBar)) {
            Pass(TheSongScheduler.mTick, mCursor);
        }
    }
    ScheduleNext();
}

void GemCatcher::CatchCheckCmd::Restart() {
    TheSongScheduler.Cancel(this);
    mId = TheDefaultCommandId;
    Schedule();
}

void GemCatcher::CatchCheckCmd::Catch() {
    const int nTick = TheSongScheduler.mTick;
    if (!mCursor.IsValid()) {
        Miss(nTick);
        return;
    }

    // The press may catch the gem the command waits on or the next gem of the lane, whichever is
    // nearer, as long as its bar is open.
    GemCursor current(mCursor);
    GemCursor next(mCursor);
    next.AdvanceToLane(mLane);
    int nBar = current.GetTick() / mTicksPerBar;
    if (mState->IsCaptured(nBar) || !mState->IsEnabled(nBar)) {
        current = GemCursor();
    }
    if (next.IsValid()) {
        nBar = next.GetTick() / mTicksPerBar;
        if (mState->IsCaptured(nBar) || !mState->IsEnabled(nBar)) {
            next = GemCursor();
        }
    }

    const GemCursor nearest = Nearest(current, next, nTick);
    if (!nearest.IsValid()) {
        Miss(nTick);
        return;
    }
    const float fMsPerTick = *mMsPerTick;
    const float fErrorMs =
        static_cast<float>(nearest.GetTick()) * fMsPerTick - static_cast<float>(nTick) * fMsPerTick;
    if (std::fabs(fErrorMs) < static_cast<float>(mSlopMs)) {
        Hit(nTick, nearest, fErrorMs);
        TheSongScheduler.Cancel(this);
        if (!(nearest == mCursor)) {
            Pass(nTick, mCursor);
            mCursor.AdvanceToLane(mLane);
        }
        ScheduleNext();
    } else {
        Miss(nTick, nearest, fErrorMs);
    }
}

void GemCatcher::CatchCheckCmd::ScheduleNext() {
    if (!mCursor.IsValid()) {
        Restart();
        return;
    }
    (void)mCursor.GetTick(); // Yes, the binary discards this call's result.
    mCursor.AdvanceToLane(mLane);
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAtTime(this,
                                    static_cast<float>(mCursor.GetTick()) * *mMsPerTick +
                                        static_cast<float>(mSlopMs),
                                    mId,
                                    false);
    }
}

GemCursor GemCatcher::CatchCheckCmd::Nearest(const GemCursor &first,
                                             const GemCursor &second,
                                             int nTick) const {
    if (!first.IsValid() || !second.IsValid()) {
        return first.IsValid() ? first : second;
    }
    if (std::abs(first.GetTick() - nTick) < std::abs(second.GetTick() - nTick)) {
        return first;
    }
    return second;
}

void GemCatcher::CatchCheckCmd::Hit(int nTick, const GemCursor &cursor, float fErrorMs) {
    TheStats->GemHit(mTrack, cursor.GetTick(), nTick, fErrorMs);
    mReceiver->OnHit(nTick, cursor);
}

void GemCatcher::CatchCheckCmd::Pass(int nTick, const GemCursor &cursor) {
    TheStats->GemPass(mTrack, cursor.GetTick());
    mReceiver->OnPass(nTick, cursor);
}

void GemCatcher::CatchCheckCmd::Miss(int nTick, const GemCursor &cursor, float fErrorMs) {
    TheStats->GemMiss(mTrack, cursor.GetTick(), nTick, fErrorMs);
    mReceiver->OnMiss(nTick, mLane);
}

void GemCatcher::CatchCheckCmd::Miss(int nTick) {
    TheStats->GemMiss(mTrack, nTick);
    mReceiver->OnMiss(nTick, mLane);
}

GemCatcher::GemCatcher(Receiver *pReceiver,
                       CatchTrackState *pState,
                       const float *pMsPerTick,
                       int nTrack,
                       int nTicksPerBar) {
    for (int nLane = 0; nLane < kNumLanes; ++nLane) {
        mChecks[nLane] = Ptr<CatchCheckCmd>(new CatchCheckCmd(
            pReceiver, pState, pMsPerTick, nTrack, nLane, TheGameConfig->mSlopMs, nTicksPerBar));
    }
}

GemCatcher::~GemCatcher() {
    Stop();
}

void GemCatcher::Start() {
    Stop();
    for (Ptr<CatchCheckCmd> &check : mChecks) {
        check->Schedule();
    }
}

void GemCatcher::Stop() {
    for (Ptr<CatchCheckCmd> &check : mChecks) {
        TheSongScheduler.Cancel(check.Get());
    }
}

void GemCatcher::Reset() {
    for (Ptr<CatchCheckCmd> &check : mChecks) {
        check->Restart();
    }
}

void GemCatcher::Catch(int nLane) {
    mChecks[nLane]->Catch();
}
