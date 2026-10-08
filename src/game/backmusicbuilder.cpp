#include "game/backmusicbuilder.h"

namespace {

// The validator checks the messages of every channel.
constexpr int kAnyChannel = -1;

} // namespace

BackMusicBuilder::BackMusicBuilder(int nTrack,
                                   bool bValidate,
                                   ErrorHandler pfnError,
                                   int nIntroBars,
                                   int nNumBars,
                                   int nTicksPerBar,
                                   BackMusic *pMusic)
    : TrackBuilder(nTrack, bValidate, pfnError), mIntroBars(nIntroBars), mNumBars(nNumBars),
      mTicksPerBar(nTicksPerBar), mMusic(pMusic), mValidator(nTrack, pfnError, kAnyChannel, true) {
}

BackMusicBuilder::~BackMusicBuilder() {
}

void BackMusicBuilder::OnEndTrack() {
    for (int i = 0; i < mNumBars + mIntroBars; ++i) {
        mMusic->SetBar(i - mIntroBars, mPieces.Extract(i * mTicksPerBar, (i + 1) * mTicksPerBar));
    }
}

void BackMusicBuilder::OnMidi(int nTick,
                              unsigned char nStatus,
                              unsigned char nData1,
                              unsigned char nData2) {
    if (mValidate) {
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }
    mPieces.AddMidi(nTick, nStatus, nData1, nData2);
}
