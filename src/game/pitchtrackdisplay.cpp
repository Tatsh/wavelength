#include "game/pitchtrackdisplay.h"

#include "game/gemslice.h"
#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr int kNoOwner = -1;
constexpr int kNoPlayer = 0;
constexpr signed char kNoRiff = -1;
constexpr int kNoFlags = 0;
constexpr int kDefaultStyle = 0;

// The flags of a bar that starts a section of the song.
constexpr int kSectionStartFlags = 12;

// The display runs this many ticks ahead of the song.
constexpr int kLookaheadTicks = 13000;

// A gem repeated through its section lies on every other bar.
constexpr int kRepeatBars = 2;

} // namespace

PitchTrackDisplay::PitchTrackDisplay(int nTrack,
                                     int nLeadInBars,
                                     int nTicksPerBar,
                                     PlayMap *pPlayMap,
                                     SectionBoundaries *pSections,
                                     PitchTrackPlayGems *pGems)
    : mTrack(nTrack), mTicksPerBar(nTicksPerBar), mPlayMap(pPlayMap), mSections(pSections),
      mGems(pGems), mUpdateCommand(NewMemFunCommand(this, &PitchTrackDisplay::Update)),
      mEndBar(-nLeadInBars), mStyle(kDefaultStyle) {
}

PitchTrackDisplay::~PitchTrackDisplay() {
    Stop();
}

void PitchTrackDisplay::Start() {
    Update();
}

void PitchTrackDisplay::Stop() {
    TheSongScheduler.Cancel(mUpdateCommand.Get());
}

void PitchTrackDisplay::SetGems(PitchTrackPlayGems *pGems, int nStyle, int nUnused, int nOwner) {
    mStyle = nStyle;
    mGems = pGems;
    DrawGems(TheSongScheduler.mTick / mTicksPerBar, mEndBar, true, nUnused, nOwner);
}

void PitchTrackDisplay::BlankBars(int nStartBar, int nEndBar) {
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

void PitchTrackDisplay::ShowGem(signed char nSlot, int nTick, bool bRepeat, int nOwner) {
    DrawGem(nSlot, nTick, bRepeat, nOwner, true);
}

void PitchTrackDisplay::HideGem(int nTick, bool bRepeat) {
    DrawGem(-1, nTick, bRepeat, kNoOwner, false);
}

void PitchTrackDisplay::Redraw(int nStartBar) {
    Redraw(nStartBar, mEndBar);
}

void PitchTrackDisplay::Redraw(int nStartBar, int nEndBar) {
    DrawRange(nStartBar, mEndBar < nEndBar ? mEndBar : nEndBar, true);
}

void PitchTrackDisplay::ClearGems(int nStartBar) {
    ClearGems(nStartBar, mEndBar);
}

void PitchTrackDisplay::ClearGems(int nStartBar, int nEndBar) {
    TheGfxManager.ClearGems(mTrack,
                            true,
                            static_cast<float>(nStartBar * mTicksPerBar),
                            static_cast<float>(nEndBar * mTicksPerBar));
}

void PitchTrackDisplay::RedrawBar(int nBar, bool bClear) {
    if (bClear) {
        ClearGems(nBar, nBar + 1);
    } else {
        DrawGems(nBar, nBar + 1, true, 0, kNoOwner);
    }
    const int nSongBar = mPlayMap->MapBar(nBar);
    // Yes, the binary processes nBar a second time as the first bar of this loop.
    for (int nLaterBar = nBar; nLaterBar < mEndBar; ++nLaterBar) {
        if (mPlayMap->MapBar(nLaterBar) == nSongBar) {
            if (bClear) {
                ClearGems(nLaterBar, nLaterBar + 1);
            } else {
                DrawGems(nLaterBar, nLaterBar + 1, true, 0, kNoOwner);
            }
        }
    }
}

void PitchTrackDisplay::ClearRepeatedBars(int nBar) {
    const int nSection = mSections->SectionAt(mPlayMap->MapBar(nBar));
    for (int nLaterBar = nBar; nLaterBar < mEndBar; nLaterBar += kRepeatBars) {
        if (mSections->SectionAt(mPlayMap->MapBar(nLaterBar)) != nSection) {
            break;
        }
        RedrawBar(nLaterBar, true);
    }
}

void PitchTrackDisplay::ClearBar(int nBar, bool bRepeat) {
    if (bRepeat) {
        ClearRepeatedBars(nBar);
    } else {
        RedrawBar(nBar, true);
    }
}

void PitchTrackDisplay::DrawGem(
    signed char nSlot, int nTick, bool bRepeat, int nOwner, bool bPlace) {
    TheGfxManager.ClearGems(
        mTrack, !bPlace, static_cast<float>(nTick), static_cast<float>(nTick + 1));
    if (bPlace) {
        TheGfxManager.PlaceGem(mTrack, nSlot, nOwner, static_cast<float>(nTick), 0, 0);
    }
    if (!bRepeat) {
        return;
    }

    const int nBar = nTick / mTicksPerBar;
    const int nSection = mSections->SectionAt(mPlayMap->MapBar(nBar));
    const int nOffset = nTick % mTicksPerBar;
    for (int nLaterBar = nBar + kRepeatBars; nLaterBar < mEndBar; nLaterBar += kRepeatBars) {
        if (mSections->SectionAt(mPlayMap->MapBar(nLaterBar)) != nSection) {
            break;
        }
        const int nLaterTick = nLaterBar * mTicksPerBar + nOffset;
        TheGfxManager.ClearGems(
            mTrack, !bPlace, static_cast<float>(nLaterTick), static_cast<float>(nLaterTick + 1));
        if (bPlace) {
            TheGfxManager.PlaceGem(mTrack, nSlot, nOwner, static_cast<float>(nLaterTick), 0, 0);
        }
    }
}

void PitchTrackDisplay::Update() {
    const int nEndBar = (TheSongScheduler.mTick + kLookaheadTicks) / mTicksPerBar;
    DrawRange(mEndBar, nEndBar, false);
    mEndBar = nEndBar;
    TheSongScheduler.PostIn(mUpdateCommand.Get(), mTicksPerBar, false);
}

void PitchTrackDisplay::DrawRange(int nStartBar, int nEndBar, bool bClear) {
    DrawBars(nStartBar, nEndBar);
    DrawGems(nStartBar, nEndBar, bClear, 0, kNoOwner);
}

void PitchTrackDisplay::DrawBars(int nStartBar, int nEndBar) {
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        const int nSongBar = mPlayMap->MapBar(nBar);
        if (nSongBar < 0) {
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
            const bool bSectionStart =
                mSections->SectionStart(mSections->SectionAt(nSongBar)) == nSongBar;
            TheGfxManager.SetBar(mTrack,
                                 kNoPlayer,
                                 kNoOwner,
                                 true,
                                 true,
                                 bSectionStart ? kSectionStartFlags : kNoFlags,
                                 kNoRiff,
                                 static_cast<float>(nBar * mTicksPerBar),
                                 static_cast<float>(mTicksPerBar));
        }
    }
}

void PitchTrackDisplay::DrawGems(
    int nStartBar, int nEndBar, bool bClear, [[maybe_unused]] int nUnused, int nOwner) {
    const int nFirstBar = nStartBar < 0 ? 0 : nStartBar;
    if (bClear) {
        TheGfxManager.ClearGems(mTrack,
                                false,
                                static_cast<float>(nFirstBar * mTicksPerBar),
                                static_cast<float>(nEndBar * mTicksPerBar));
    }
    for (int nBar = nFirstBar; nBar < nEndBar; ++nBar) {
        if (mPlayMap->MapBar(nBar) < 0) {
            continue;
        }
        const GemSlice gems = mGems->GetBar(nBar);
        for (int i = 0; i < gems.Size(); ++i) {
            const int nTick = gems.TickAt(i);
            const int nGemOwner = nOwner == kNoOwner ? gems.At(i).mOwner : nOwner;
            TheGfxManager.PlaceGem(
                mTrack, gems.At(i).mSlot, nGemOwner, static_cast<float>(nTick), mStyle, 0);
        }
    }
}
