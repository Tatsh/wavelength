#include "game/voxtrackmusic.h"

#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr int kNoBar = -1;
constexpr int kMultiMuseValue = 0;

} // namespace

VoxTrackMusic::VoxTrackMusic(CatchTrackData *pGems,
                             PlayMap *pPlayMap,
                             int nTicksPerBar,
                             int nNumBars)
    : mTicksPerBar(nTicksPerBar), mPlayMap(pPlayMap),
      mBarCommand(NewMemFunCommand(this, &VoxTrackMusic::StartBar)), mBars(nNumBars, nullptr),
      mBar(0), mLastBar(kNoBar), mMuted(0) {
    BuildBars(pGems, nNumBars);
}

VoxTrackMusic::~VoxTrackMusic() {
    Stop();
}

void VoxTrackMusic::Start() {
    TheSongScheduler.PostAt(mBarCommand.Get(), 0, false);
}

void VoxTrackMusic::Stop() {
    TheSongScheduler.Cancel(mBarCommand.Get());
}

void VoxTrackMusic::SetMuted(bool bMuted) {
    mMuted = bMuted;
    const int nNow = TheSongScheduler.mTick;
    if (nNow < 0) {
        return;
    }
    // Yes, the binary skips bar 0 here where mBar includes it.
    if (bMuted) {
        if (mLastBar > 0) {
            mBars[mLastBar]->Stop();
        }
        if (mBar >= 0) {
            mBars[mBar]->Stop();
        }
        return;
    }
    const int nOffset = nNow % mTicksPerBar;
    const int nSongBar = mPlayMap->MapBar(nNow / mTicksPerBar);
    if (nSongBar > 0) {
        mBars[nSongBar]->PlayFrom(&TheSongScheduler, nOffset);
    }
}

void VoxTrackMusic::StartBar() {
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    mLastBar = mBar;
    mBar = mPlayMap->MapBar(nBar);
    if (mBar != mLastBar + 1) {
        // Yes, the binary does not check that mLastBar is a bar.
        mBars[mLastBar]->Stop();
    }
    if (mMuted == 0 && mBar >= 0) {
        mBars[mBar]->Play(&TheSongScheduler);
    }
    TheSongScheduler.PostAt(mBarCommand.Get(), (nBar + 1) * mTicksPerBar, false);
}

void VoxTrackMusic::BuildBars(CatchTrackData *pGems, int nNumBars) {
    const int nNumGems = pGems->GetNumGems();
    int nGem = 0;
    while (nGem < nNumGems && pGems->GetGem(nGem)->mTick < 0) {
        ++nGem;
    }
    for (int nBar = 0; nBar < nNumBars; ++nBar) {
        auto *pBar = new MultiMuse(kMultiMuseValue);
        mBars[nBar] = pBar;
        const int nStartTick = nBar * mTicksPerBar;
        const int nEndTick = (nBar + 1) * mTicksPerBar;
        int nTick = 0;
        while (nGem < nNumGems && (nTick = pGems->GetGem(nGem)->mTick) < nEndTick) {
            pBar->Add(pGems->GetGem(nGem)->mMuse.Get(), nTick - nStartTick);
            ++nGem;
        }
    }
}
