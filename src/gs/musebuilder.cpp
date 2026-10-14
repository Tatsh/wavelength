#include "gs/musebuilder.h"

#include "gs/multimuse.h"

namespace {

constexpr bool kRequirePrograms = true;

} // namespace

MuseBuilder::MuseBuilder(int nTrack, bool bValidate, ErrorHandler pfnError, Ptr<Muse> *pMuse)
    : TrackBuilder(nTrack, bValidate, pfnError), mMuse(pMuse),
      mValidator(nTrack, pfnError, MidiValidator::kNone, kRequirePrograms) {
}

MuseBuilder::~MuseBuilder() {
}

void MuseBuilder::OnEndTrack() {
    const Ptr<Muse> piece(mPieces.ExtractAll());
    *mMuse = piece;
}

void MuseBuilder::OnMidi(int nTick,
                         unsigned char nStatus,
                         unsigned char nData1,
                         unsigned char nData2) {
    if (mValidate) {
        mValidator.Check(nTick, nStatus, nData1, nData2);
    }
    mPieces.AddMidi(nTick, nStatus, nData1, nData2);
}
