#include "game/voxtrackdisplay.h"

#include "game/gemcursor.h"
#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr int kNoOwner = -1;
constexpr int kFirstOwner = 0;
constexpr int kNoPlayer = 0;
constexpr signed char kNoRiff = -1;
constexpr int kNoFlags = 0;
constexpr int kDefaultStyle = 0;

// The display runs this many ticks ahead of the song.
constexpr int kLookaheadTicks = 13000;

} // namespace

VoxTrackDisplay::VoxTrackDisplay(
    int nTrack, int nLeadInBars, int nTicksPerBar, PlayMap *pPlayMap, CatchTrackData *pGems)
    : mTrack(nTrack), mTicksPerBar(nTicksPerBar), mPlayMap(pPlayMap), mGems(pGems),
      mUpdateCommand(NewMemFunCommand(this, &VoxTrackDisplay::Update)), mEndBar(-nLeadInBars) {
}

VoxTrackDisplay::~VoxTrackDisplay() {
    Stop();
}

void VoxTrackDisplay::Start() {
    Update();
}

void VoxTrackDisplay::Stop() {
    TheSongScheduler.Cancel(mUpdateCommand.Get());
}

void VoxTrackDisplay::BlankBars(int nStartBar, int nEndBar) {
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        TheGfxManager.SetBar(mTrack,
                             kNoPlayer,
                             kNoOwner,
                             false,
                             true,
                             kNoFlags,
                             kNoRiff,
                             static_cast<float>(nBar * mTicksPerBar),
                             static_cast<float>(mTicksPerBar));
    }
    TheGfxManager.ClearGems(mTrack,
                            false,
                            static_cast<float>(nStartBar * mTicksPerBar),
                            static_cast<float>(nEndBar * mTicksPerBar));
}

void VoxTrackDisplay::Redraw(int nStartBar) {
    Redraw(nStartBar, mEndBar);
}

void VoxTrackDisplay::Redraw(int nStartBar, int nEndBar) {
    DrawRange(nStartBar, mEndBar < nEndBar ? mEndBar : nEndBar, true);
}

void VoxTrackDisplay::Update() {
    const int nEndBar = (TheSongScheduler.mTick + kLookaheadTicks) / mTicksPerBar;
    DrawRange(mEndBar, nEndBar, false);
    mEndBar = nEndBar;
    TheSongScheduler.PostIn(mUpdateCommand.Get(), mTicksPerBar, false);
}

void VoxTrackDisplay::DrawRange(int nStartBar, int nEndBar, bool bClear) {
    DrawBars(nStartBar, nEndBar);
    DrawGems(nStartBar, nEndBar, bClear);
}

void VoxTrackDisplay::DrawBars(int nStartBar, int nEndBar) {
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        if (nBar < 0) {
            TheGfxManager.SetBar(mTrack,
                                 kNoPlayer,
                                 kNoOwner,
                                 false,
                                 true,
                                 kNoFlags,
                                 kNoRiff,
                                 static_cast<float>(nBar * mTicksPerBar),
                                 static_cast<float>(mTicksPerBar));
        } else {
            TheGfxManager.SetBar(mTrack,
                                 kNoPlayer,
                                 kFirstOwner,
                                 true,
                                 true,
                                 kNoFlags,
                                 kNoRiff,
                                 static_cast<float>(nBar * mTicksPerBar),
                                 static_cast<float>(mTicksPerBar));
        }
    }
}

void VoxTrackDisplay::DrawGems(int nStartBar, int nEndBar, bool bClear) {
    const int nFirstBar = nStartBar < 0 ? 0 : nStartBar;
    const int nStartTick = nFirstBar * mTicksPerBar;
    const int nEndTick = nEndBar * mTicksPerBar;
    if (bClear) {
        TheGfxManager.ClearGems(
            mTrack, false, static_cast<float>(nStartTick), static_cast<float>(nEndTick));
    }
    GemCursor cursor(mPlayMap, mGems, nStartTick);
    while (cursor.IsValid() && cursor.GetTick() < nEndTick) {
        const float fTick = static_cast<float>(cursor.GetTick());
        TheGfxManager.PlaceGem(mTrack, cursor.GetLane(), kNoPlayer, fTick, kDefaultStyle, kNoFlags);
        cursor.Next();
    }
}
