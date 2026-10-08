#include "game/conductorbuilder.h"

#include <cmath>

namespace {

// The length of a beat in ticks that the tempo check assumes, whatever mTicksPerBeat is.
constexpr float kCheckTicksPerBeat = 480.0f;

constexpr float kMicrosecondsPerMillisecond = 1000.0f;
constexpr double kMaxTempoErrorMs = 0.1;

} // namespace

ConductorBuilder::ConductorBuilder(int nTrack,
                                   bool bValidate,
                                   ErrorHandler pfnError,
                                   int nNumBars,
                                   int nTicksPerBeat,
                                   const float *pfMsPerTick,
                                   PlayMap **ppPlayMap,
                                   int *pTicksPerBar)
    : TrackBuilder(nTrack, bValidate, pfnError), mNumBars(nNumBars), mTicksPerBeat(nTicksPerBeat),
      mMsPerTick(pfMsPerTick), mPlayMap(ppPlayMap), mTicksPerBar(pTicksPerBar) {
}

ConductorBuilder::~ConductorBuilder() {
}

void ConductorBuilder::OnTempo(int nTick, int nMicrosecondsPerBeat) {
    if (!mValidate) {
        return;
    }

    const float fMsPerBeat = static_cast<float>(nMicrosecondsPerBeat) / kMicrosecondsPerMillisecond;
    const float fError = std::fabs((*mMsPerTick * kCheckTicksPerBeat) - fMsPerBeat);
    if (static_cast<double>(fError) > kMaxTempoErrorMs) {
        Error(nTick, "Tempo in midi file does not match \"bpm\" array in text file");
    }
}

void ConductorBuilder::OnTimeSignature(int nTick,
                                       int nNumerator,
                                       [[maybe_unused]] int nDenominator) {
    if (*mTicksPerBar > 0) {
        Error(nTick, "time signature set more than once; variable time signature not supported.");
    }
    *mTicksPerBar = nNumerator * mTicksPerBeat;
    *mPlayMap = new PlayMap(*mTicksPerBar, mNumBars);
}
