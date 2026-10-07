#include "game/catchtrackmusic.h"

#include "game/gameconfig.h"

namespace {

// Start() never schedules the first bar before tick 0.
constexpr int kFirstTick = 0;

} // namespace

CatchTrackMusic::CatchTrackMusic(
    CatchTrackState *pState, int nTicksPerBar, int nNumBars, PlayMap *pPlayMap, int nStopPrevious)
    : mState(pState), mPlayBarCommand(new PlayBarCmd(this)), mBars(nNumBars),
      mTicksPerBar(nTicksPerBar), mPlayMap(pPlayMap), mStopPrevious(nStopPrevious),
      mCurrent(nullptr), mPrevious(nullptr) {
    BuildBars(nNumBars);
}

CatchTrackMusic::~CatchTrackMusic() {
    Stop();
}

void CatchTrackMusic::Start() {
    Stop();
    int nTick = TheSongScheduler.mTick;
    if (nTick < kFirstTick) {
        nTick = kFirstTick;
    }
    if (nTick % mTicksPerBar != 0) {
        nTick = ((nTick / mTicksPerBar) * mTicksPerBar) + mTicksPerBar;
    }
    TheSongScheduler.PostAt(mPlayBarCommand.Get(), nTick, false);
}

void CatchTrackMusic::Stop() {
    TheSongScheduler.Cancel(mPlayBarCommand.Get());
    for (const Ptr<MultiMuse> &bar : mBars) {
        bar->Stop();
    }
}

void CatchTrackMusic::Refresh() {
    const int nTick = TheSongScheduler.mTick;
    const int nBar = nTick / mTicksPerBar;
    const int nWrittenBar = mPlayMap->MapBar(nBar);
    const int nBarStart = nWrittenBar * mTicksPerBar;
    Muse *pBar = mBars[nWrittenBar].Get();
    const bool bCaptured = mState->GetCapturedBy(nBar) != nullptr;
    const bool bPlaying = pBar->IsPlaying();
    if (bCaptured) {
        if (!bPlaying) {
            mPrevious = mCurrent;
            mCurrent = pBar;
            pBar->PlayFrom(&TheSongScheduler, nTick - nBarStart);
        }
    } else if (bPlaying) {
        pBar->Stop();
    }
}

void CatchTrackMusic::BuildBars(int nNumBars) {
    CatchTrackData *pGems = mState->GetData();
    const int nNumGems = pGems->GetNumGems();
    int nGem = 0;
    while (nGem < nNumGems && pGems->GetGem(nGem)->mTick < 0) {
        ++nGem;
    }
    for (int nBar = 0; nBar < nNumBars; ++nBar) {
        auto *pBar = new MultiMuse(mStopPrevious);
        mBars[nBar] = Ptr<MultiMuse>(pBar);
        const int nStartTick = nBar * mTicksPerBar;
        const int nEndTick = (nBar + 1) * mTicksPerBar;
        int nTick = 0;
        while (nGem < nNumGems && (nTick = pGems->GetGem(nGem)->mTick) < nEndTick) {
            Muse *pMuse = pGems->GetGem(nGem++)->mMuse.Get();
            pBar->Add(pMuse, nTick - nStartTick);
        }
        if (mStopPrevious != 0) {
            pBar->SetNoteCB(this);
        }
    }
}

void CatchTrackMusic::PlayBar() {
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    const int nEndBar = mPlayMap->GetEndBar();
    Muse *pBar;
    if (nBar < nEndBar) {
        if (mState->GetCapturedBy(nBar) == nullptr && TheGameConfig->mPlayAllGems == 0) {
            return;
        }
        pBar = mBars[mPlayMap->MapBar(nBar)].Get();
        if (pBar->IsPlaying()) {
            return;
        }
    } else {
        // Past the end of the song every bar plays, whether or not its phrase is captured.
        pBar = mBars[nBar % nEndBar].Get();
    }
    mPrevious = mCurrent;
    mCurrent = pBar;
    pBar->Play(&TheSongScheduler);
}

void CatchTrackMusic::OnNote([[maybe_unused]] unsigned char nNote, [[maybe_unused]] int nDuration) {
    if (mPrevious != nullptr) {
        mPrevious->Stop();
        mPrevious = nullptr;
    }
}
