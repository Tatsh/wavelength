#include "game/guideticker.h"

#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kMutedBars = 2;

} // namespace

GuideTicker::GuideTicker(CatchTrackState *pState, int nTicksPerBar)
    : mBarsLeft(0), mMuted(0), mState(pState), mCursor(pState->GetCursor()),
      mUpdateCmd(NewMemFunCommand(this, &GuideTicker::Update)) {
    mTicksPerBar = nTicksPerBar;
    mLastBar = -1;
}

GuideTicker::~GuideTicker() {
    Stop();
}

void GuideTicker::Start() {
    TheSongScheduler.PostIn(mUpdateCmd.Get(), 0, false);
}

void GuideTicker::Stop() {
    TheSongScheduler.Cancel(mUpdateCmd.Get());
}

void GuideTicker::Mute() {
    mBarsLeft = 0;
    mMuted = 1;
}

void GuideTicker::Unmute() {
    mMuted = 0;
}

void GuideTicker::MuteForTwoBars() {
    Mute();
    mBarsLeft = kMutedBars;
    mLastBar = TheSongScheduler.mTick / mTicksPerBar;
}

void GuideTicker::Update() {
    while (mCursor.IsValid() && mCursor.GetTick() <= TheSongScheduler.mTick) {
        const int nBar = mCursor.GetTick() / mTicksPerBar;
        if (mState->IsEnabled(nBar) && !mState->GetCapturedBy(nBar)) {
            if (mMuted && mLastBar < nBar && mBarsLeft > 0) {
                mLastBar = nBar;
                if (nBar > 0) { // Bar 0 does not count, as in the binary.
                    --mBarsLeft;
                }
                if (mBarsLeft == 0) {
                    Unmute();
                }
            }
            if (!mMuted) {
                FxMidi::PlayGuideSound(mCursor.GetLane());
            }
        }
        (void)mCursor.Next(); // The binary discards the returned copy.
    }
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAt(mUpdateCmd.Get(), mCursor.GetTick(), false);
    }
}
