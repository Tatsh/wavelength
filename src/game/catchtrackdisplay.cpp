#include "game/catchtrackdisplay.h"

#include "gfx/gfxmanager.h"
#include "os/scheduler.h"

namespace {

constexpr int kNoPlayer = -1;
constexpr signed char kNoRiff = -1;
constexpr int kMainCamera = 0;

// The ticks ahead of the current tick the display draws up to.
constexpr int kLookaheadTicks = 13000;

// The updates of each track lie a tenth of a bar apart.
constexpr int kStaggerDivisor = 10;

enum BarFlags {
    kBarFlagsNone = 0,
    kBarFlagsPowerup = 2,
};

} // namespace

void CatchTrackDisplay::UpdateDisplayCmd::Execute() {
    if (mDisplay->Advance()) {
        TheSongScheduler.PostIn(this, mDisplay->mTicksPerBar, false);
    }
}

CatchTrackDisplay::CatchTrackDisplay(CatchTrackState *pState,
                                     int nReserved04,
                                     int nTrack,
                                     int nReserved0C,
                                     int nReserved10,
                                     int nTicksPerBar,
                                     PlayMap *pPlayMap)
    : mReserved04(nReserved04), mTrack(nTrack), mReserved0C(nReserved0C), mReserved10(nReserved10),
      mTicksPerBar(nTicksPerBar), mUpdateCmd(new UpdateDisplayCmd(this)), mPlayMap(pPlayMap) {
    mState = pState;
    mDrawnBar = 0;
    mTickOffset = 0;
}

CatchTrackDisplay::~CatchTrackDisplay() {
    Stop();
}

void CatchTrackDisplay::Start() {
    Stop();
    ScheduleUpdate();
}

void CatchTrackDisplay::Seek(int nTick) {
    Stop();
    mTickOffset = nTick - TheSongScheduler.mTick;
    mDrawnBar = nTick / mTicksPerBar - 1;
    (void)Advance(); // The binary discards the result.
    ScheduleUpdate();
}

void CatchTrackDisplay::Stop() {
    TheSongScheduler.Cancel(mUpdateCmd.Get());
}

void CatchTrackDisplay::Redraw(int nFromBar, int nClear) {
    RedrawRange(nFromBar, mDrawnBar, nClear);
}

void CatchTrackDisplay::RedrawRange(int nFromBar, int nToBar, int nClear) {
    const int nEndBar = mPlayMap->GetEndBar();
    if (nEndBar < nToBar) {
        nToBar = nEndBar;
    }
    if (!(nFromBar < nToBar)) {
        return;
    }
    if (mDrawnBar < nToBar) {
        nToBar = mDrawnBar;
    }
    if (nClear) {
        TheGfxManager.ClearGems(mTrack,
                                false,
                                static_cast<float>(nFromBar * mTicksPerBar),
                                static_cast<float>(nToBar * mTicksPerBar));
    }
    DrawBars(nFromBar, nToBar, 0);
}

void CatchTrackDisplay::ScheduleUpdate() {
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    const int nStagger = (mTrack + 1) * mTicksPerBar / kStaggerDivisor;
    TheSongScheduler.PostAt(mUpdateCmd.Get(), nBar * mTicksPerBar + mTicksPerBar + nStagger, false);
}

void CatchTrackDisplay::PlaceGems(int nFromBar, int nToBar) {
    GemCursor cursor = mState->GetCursor(nFromBar * mTicksPerBar);
    const int nEndTick = nToBar * mTicksPerBar;
    int nLastBar = -1;
    int nPowerup = 0;
    while (cursor.IsValid()) {
        const int nTick = cursor.GetTick();
        if (!(nTick < nEndTick)) {
            break;
        }
        const int nBar = nTick / mTicksPerBar;
        if (nBar != nLastBar) {
            nLastBar = nBar;
            nPowerup = mState->GetPowerup(nBar);
        }
        Player *pCapturedBy = mState->GetCapturedBy(nBar);
        const int nPlayer = pCapturedBy != nullptr ? pCapturedBy->GetIndex() : kNoPlayer;
        if (mState->IsEnabled(nBar) || pCapturedBy != nullptr) {
            TheGfxManager.PlaceGem(
                mTrack, cursor.GetLane(), nPlayer, static_cast<float>(nTick), 0, nPowerup);
        }
        (void)cursor.Next(); // The binary discards the returned copy.
    }
}

void CatchTrackDisplay::DrawBars(int nFromBar, int nToBar, int nVisible) {
    for (int nBar = nFromBar; nBar < nToBar; ++nBar) {
        const float fTick = static_cast<float>(nBar * mTicksPerBar);
        const float fTicks = static_cast<float>(mTicksPerBar);
        if (nBar < 0 || nBar == mPlayMap->GetEndBar()) {
            TheGfxManager.SetBar(
                mTrack, kMainCamera, kNoPlayer, false, true, kBarFlagsNone, kNoRiff, fTick, fTicks);
            continue;
        }
        Player *pCapturedBy = mState->GetCapturedBy(nBar);
        const bool bEnabled = mState->IsEnabled(nBar);
        int nFlags = kBarFlagsNone;
        if (mState->GetPowerup(nBar) != 0 && !mState->IsBarEmpty(nBar)) {
            nFlags = kBarFlagsPowerup;
        }
        TheGfxManager.SetBar(mTrack,
                             kMainCamera,
                             pCapturedBy != nullptr ? pCapturedBy->GetIndex() : kNoPlayer,
                             bEnabled,
                             nVisible != 0,
                             nFlags,
                             kNoRiff,
                             fTick,
                             fTicks);
    }
    PlaceGems(nFromBar, nToBar);
}

bool CatchTrackDisplay::Advance() {
    int nTarget = (TheSongScheduler.mTick + mTickOffset + kLookaheadTicks) / mTicksPerBar;
    const int nFirstBar = mDrawnBar;
    const int nLimit = mPlayMap->GetEndBar() + 1;
    if (!(nFirstBar < nLimit)) {
        return false;
    }
    if (nLimit < nTarget) {
        nTarget = nLimit;
    }
    DrawBars(nFirstBar, nTarget, 1);
    mDrawnBar = nTarget;
    return true;
}
