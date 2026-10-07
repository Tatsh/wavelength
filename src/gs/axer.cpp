#include "gs/axer.h"

#include <cmath>

#include "synth/synth.h"

namespace {

constexpr float kFullOutputLevel = 1.0f;
constexpr float kSweepThreshold = 0.02f;
constexpr float kHalf = 0.5f;

// The share of the range of the harmony the contour may move by.
constexpr float kContourSpread = 1.75f;

} // namespace

Axer::Axer(AxeContour *pContour, const AxeHarmony *pHarmony, int nReserved, float fOutputLevel)
    : mHarmony(pHarmony), mReserved08(nReserved), mOutputLevel(fOutputLevel),
      mChannel(pContour->GetChannel()), mTranspose(0), mSweep(0.0f), mActive(0) {
    mContour = pContour;
}

Axer::~Axer() {
    Deactivate();
}

void Axer::SetContour(AxeContour *pContour) {
    mChannel = pContour->GetChannel();
    if (mContour->IsPlaying()) {
        pContour->Play(mContour->GetPosition());
    }
    mContour->Stop();
    FitContour(mTranspose); // The binary fits the previous contour.
    mContour = pContour;
}

void Axer::SetHarmony(const AxeHarmony *pHarmony) {
    mHarmony = pHarmony;
    FitContour(mTranspose);
}

int Axer::GetLength() {
    return mContour->GetLength();
}

void Axer::Activate(int nPosition) {
    if (mActive) {
        return;
    }
    mActive = 1;
    FitContour(mTranspose);
    ApplySweep(mSweep);
    ApplyReserved();
    TheSynth->SetOutputLevel(mOutputLevel);
    mContour->Play(nPosition);
}

void Axer::Deactivate() {
    if (!mActive) {
        return;
    }
    mActive = 0;
    TheSynth->SetOutputLevel(kFullOutputLevel);
    mContour->Stop();
}

int Axer::IsActive() const {
    return mActive;
}

void Axer::SetX(float fX) {
    TheSynth->VirtualSlot26(fX);
    const int nContourRange = mContour->GetHighestKey() - mContour->GetLowestKey();
    const int nHarmonyRange = mHarmony->GetHighest() - mHarmony->GetLowest();
    const int nTranspose = static_cast<int>(
        (static_cast<float>(nHarmonyRange) - static_cast<float>(nContourRange) * kContourSpread) *
        kHalf * fX);
    if (nTranspose != mTranspose) {
        mTranspose = nTranspose;
        FitContour(nTranspose);
    }
}

void Axer::SetSweep(float fSweep) {
    if (kSweepThreshold < std::fabs(fSweep - mSweep)) {
        mSweep = fSweep;
        ApplySweep(fSweep);
    }
}

void Axer::FitContour(int nTranspose) {
    mHarmony->SnapContour(mContour, nTranspose);
}

void Axer::ApplySweep(float fSweep) {
    TheSynth->SetSoftFxSweep(fSweep);
}

void Axer::ApplyReserved() {
}
