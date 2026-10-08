#include "gfx/tnlwave.h"

#include "math/sine.h"

namespace {

constexpr float kTwoPi = 6.28318548f;

} // namespace

TnlWave::TnlWave(const char *pszName) : mName(pszName) {
}

TnlWave::TnlWave(float flAmplitude, float flFrequency, float flPhase)
    : mName(static_cast<const char *>(nullptr)), mAmplitude(flAmplitude), mFrequency(flFrequency),
      mPhase(flPhase) {
}

TnlWave::~TnlWave() {
}

void TnlWave::Sync(float flTime, float flPhase) {
    mPhase = flPhase - ((mFrequency * kTwoPi) * flTime);
}

float TnlWave::Sine(float flTime) const {
    return FastSin(((mFrequency * kTwoPi) * flTime) + mPhase);
}
