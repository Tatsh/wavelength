#include "game/pitchtrackplaygems.h"

namespace {

constexpr int kNoOwner = -1;

// A gem repeated through its section lies on every other bar.
constexpr int kRepeatBars = 2;

} // namespace

PitchTrackPlayGems::PitchTrackPlayGems(PitchTrackGems *pGems,
                                       PlayMap *pPlayMap,
                                       SectionBoundaries *pSections,
                                       SlotGrid *pPatterns,
                                       [[maybe_unused]] int nNumBars,
                                       int nTicksPerBar)
    : mGems(pGems), mPlayMap(pPlayMap), mSections(pSections), mPatterns(pPatterns),
      mTicksPerBar(nTicksPerBar) {
}

PitchTrackPlayGems::~PitchTrackPlayGems() {
}

bool PitchTrackPlayGems::ToggleGem(signed char nSlot, int nTick, signed char nOwner, bool bRepeat) {
    const int nSongTick = mPlayMap->MapTick(nTick);
    const int nSongBar = nSongTick / mTicksPerBar;
    if (!bRepeat) {
        return mGems->ToggleGem(nSlot, nSongTick, nOwner);
    }

    int nStartBar = 0;
    int nEndBar = 0;
    mSections->GetSectionRange(mSections->SectionAt(nSongBar), &nStartBar, &nEndBar);
    nStartBar += (nSongBar - nStartBar) % kRepeatBars;
    const bool bPlaced = mGems->ToggleGem(nSlot, nSongTick, nOwner);
    for (int nBar = nStartBar; nBar < nEndBar; nBar += kRepeatBars) {
        if (nBar == nSongBar) {
            continue;
        }
        const int nRepeatTick = nSongTick + (nBar - nSongBar) * mTicksPerBar;
        if (bPlaced) {
            mGems->AddGem(nSlot, nRepeatTick, nOwner);
        } else {
            mGems->RemoveGem(nRepeatTick);
        }
    }
    return bPlaced;
}

int PitchTrackPlayGems::GetOwner(int nBar) {
    return mGems->GetOwner(mPlayMap->MapBar(nBar));
}

void PitchTrackPlayGems::ClearBar(int nBar, bool bRepeat) {
    const int nSongBar = mPlayMap->MapBar(nBar);
    if (!bRepeat) {
        mGems->ClearBar(nSongBar);
        return;
    }

    int nStartBar = 0;
    int nEndBar = 0;
    mSections->GetSectionRange(mSections->SectionAt(nSongBar), &nStartBar, &nEndBar);
    for (int nRepeatBar = nStartBar + (nSongBar - nStartBar) % kRepeatBars; nRepeatBar < nEndBar;
         nRepeatBar += kRepeatBars) {
        mGems->ClearBar(nRepeatBar);
    }
}

void PitchTrackPlayGems::ClearSection(int nBar) {
    mGems->ClearSection(mSections->SectionAt(mPlayMap->MapBar(nBar)));
}

void PitchTrackPlayGems::CopyBars(PitchTrackPlayGems *pSource, int nStartBar, int nEndBar) {
    mGems->CopyBarsFrom(nStartBar, nEndBar, pSource->mGems, kNoOwner);
}

void PitchTrackPlayGems::ApplyPatterns() {
    mGems->ApplyPatterns();
}

GemSlice PitchTrackPlayGems::GetBar(int nBar) {
    const int nSongBar = mPlayMap->MapBar(nBar);
    if (nSongBar < 0) {
        return GemSlice();
    }
    return GemSlice(mGems->GetBar(nSongBar), (nBar - nSongBar) * mTicksPerBar);
}

void PitchTrackPlayGems::Save(BinStream &stream) {
    mGems->Save(stream);
}

void PitchTrackPlayGems::Load(BinStream &stream) {
    mGems->Load(stream);
}
