#include "game/pitchtrackmusic.h"

#include "game/pitchgem.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

PitchTrackMusic::PitchTrackMusic(PitchTrackPlayGems *pGems,
                                 PitchTrackRiffs *pRiffs,
                                 int nTicksPerBar)
    : mTicksPerBar(nTicksPerBar), mGems(pGems), mRiffs(pRiffs), mBar(), mNextGem(0),
      mPlayGemCommand(NewMemFunCommand(this, &PitchTrackMusic::PlayGem)),
      mBarCommand(NewMemFunCommand(this, &PitchTrackMusic::StartBar)), mPlaying(nullptr),
      mMuted(false) {
}

PitchTrackMusic::~PitchTrackMusic() {
    Stop();
}

void PitchTrackMusic::Start() {
    TheSongScheduler.PostAt(mBarCommand.Get(), 0, false);
}

void PitchTrackMusic::Stop() {
    TheSongScheduler.Cancel(mPlayGemCommand.Get());
    TheSongScheduler.Cancel(mBarCommand.Get());
}

void PitchTrackMusic::SetGems(PitchTrackPlayGems *pGems) {
    mGems = pGems;
    TheSongScheduler.Cancel(mBarCommand.Get());
    TheSongScheduler.Cancel(mPlayGemCommand.Get());
    StartBar();
}

void PitchTrackMusic::Play(Muse *pRiff, int nOffset) {
    if (mPlaying != nullptr) {
        mPlaying->Stop();
    }
    pRiff->PlayFrom(&TheSongScheduler, nOffset);
    mPlaying = pRiff;
}

void PitchTrackMusic::SetMuted(bool bMuted) {
    mMuted = bMuted;
    if (bMuted && mPlaying != nullptr) {
        mPlaying->Stop();
    }
}

void PitchTrackMusic::StartBar() {
    const int nNow = TheSongScheduler.mTick;
    const int nBar = nNow / mTicksPerBar;
    if (nBar >= 0) {
        mBar = mGems->GetBar(nBar);
        mNextGem = 0;
        while (mNextGem < mBar.Size() && mBar.TickAt(mNextGem) < nNow) {
            ++mNextGem;
        }
        if (mNextGem < mBar.Size()) {
            TheSongScheduler.PostAt(mPlayGemCommand.Get(), mBar.TickAt(mNextGem), false);
        }
    }
    TheSongScheduler.PostAt(mBarCommand.Get(), (nBar + 1) * mTicksPerBar, false);
}

void PitchTrackMusic::PlayGem() {
    const int nGems = mBar.Size();
    const int nNow = TheSongScheduler.mTick;
    if (mNextGem >= nGems) {
        return;
    }
    if (mBar.TickAt(mNextGem) != nNow) {
        mNextGem = 0;
        if (nGems <= 0) {
            return;
        }
        while (mBar.TickAt(mNextGem) < nNow) {
            if (++mNextGem >= nGems) {
                break;
            }
        }
    }
    if (mNextGem >= nGems) {
        return;
    }

    if (mBar.TickAt(mNextGem) == nNow) {
        if (!mMuted) {
            const PitchGem &gem = mBar.At(mNextGem);
            if (mPlaying != nullptr) {
                mPlaying->Stop();
            }
            Muse *pRiff = mRiffs->GetRiff(gem.mSlot);
            pRiff->Play(&TheSongScheduler);
            mPlaying = pRiff;
        }
        ++mNextGem;
    }
    if (mNextGem < nGems) {
        TheSongScheduler.PostAt(mPlayGemCommand.Get(), mBar.TickAt(mNextGem), false);
    }
}
