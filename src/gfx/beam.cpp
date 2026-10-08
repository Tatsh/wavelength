#include "gfx/beam.h"

#include <cstdint>

#include "game/gamedb.h"
#include "math/rand.h"
#include "math/sine.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The times of a beam that is not running.
constexpr float kNever = -1e9f;

// The phase of a wave advances this many radians per millisecond, divided by its period.
constexpr float kPhasePerMillisecond = 6.0f;

constexpr float kPi = 3.14159274f;

// The sizes of the first and the second wave, as multiples of the amplitude.
constexpr float kFirstWaveScale = 0.2f;
constexpr float kSecondWaveScale = 0.1f;

// The components of a position.
constexpr int kNumComponents = 3;

} // namespace

Beam::Beam() {
    mParams = nullptr;
    mLine = nullptr;
    mMat = nullptr;
    mFrequencies.x = 0.0f;
    mFrequencies.y = 0.0f;
    mEndTime = kNever;
    mTravelPeriods.y = 1.0f;
    mStartTime = kNever;
    mTravelPeriods.x = 1.0f;
    // The names only have to be unique, and the address of the beam makes them so.
    const unsigned int nId = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(this));
    mMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Create(
        Rnd::Mat::sClassName.mStr, FormatString("beam mat 0x%08x.mat", nId)));
    mLine = dynamic_cast<Rnd::Line *>(Rnd::TheManager.Create(
        Rnd::Line::sClassName.mStr, FormatString("beam line 0x%08x.mat", nId)));
    mLine->SetMat(mMat);
    mLine->SetShowing(0);
}

Beam::~Beam() {
    delete mLine;
    delete mMat;
}

void Beam::Configure(const BeamParams *pParams) {
    mParams = pParams;
    pParams->Apply(mLine, mMat);
}

void Beam::Start(Ship *pOwner, std::list<Ship::BeamData> *pOwnerList, float fStart, float fEnd) {
    mOwnerList = pOwnerList;
    mStartTime = fStart;
    mEndTime = fEnd;
    mOwner = pOwner;
    mLine->SetShowing(1);
    const float fFrequencyA = RandomFloat(mParams->mMinFrequency.x, mParams->mMaxFrequency.x);
    const float fFrequencyB = RandomFloat(mParams->mMinFrequency.y, mParams->mMaxFrequency.y);
    mFrequencies.x = fFrequencyA;
    mFrequencies.y = fFrequencyB;
    const float fPeriodA = RandomFloat(mParams->mMinTravelPeriod.x, mParams->mMaxTravelPeriod.x);
    const float fPeriodB = RandomFloat(mParams->mMinTravelPeriod.y, mParams->mMaxTravelPeriod.y);
    mTravelPeriods.x = fPeriodA;
    mTravelPeriods.y = fPeriodB;
}

void Beam::Stop() {
    mEndTime = kNever;
    mStartTime = kNever;
    mLine->SetShowing(0);
}

bool Beam::Update(const Vector3 &direction, const Vector3 &side, const Color &color) {
    const float fNow = TheGameDb->mSongTime;
    if (mEndTime < fNow) {
        return true;
    }
    const float fT = (fNow - mStartTime) / (mEndTime - mStartTime);
    const float fAmplitude = ((mParams->mAmplitudes[BeamParams::kEndFinish] -
                               mParams->mAmplitudes[BeamParams::kEndStart]) *
                              fT) +
                             mParams->mAmplitudes[BeamParams::kEndStart];
    const float fLift[] = {
        direction.x * fAmplitude, direction.y * fAmplitude, direction.z * fAmplitude};
    const float fShift[] = {side.x * fAmplitude, side.y * fAmplitude, side.z * fAmplitude};
    const float fPhase = fNow * kPhasePerMillisecond;
    const float fPhaseB0 = fPhase / mTravelPeriods.y;
    const float fPhaseA0 = fPhase / mTravelPeriods.x;
    const float fCyclesA = (mFrequencies.x + mFrequencies.x) * kPi;
    const float fCyclesB = (mFrequencies.y + mFrequencies.y) * kPi;
    const int nPoints = mParams->mPoints;
    const float fInvPoints = 1.0f / static_cast<float>(nPoints);
    float fStep[kNumComponents];
    for (int i = 0; i < kNumComponents; ++i) {
        fStep[i] =
            (mEnds[BeamParams::kEndFinish][i] - mEnds[BeamParams::kEndStart][i]) * fInvPoints;
    }
    const float fStepA = fCyclesA * fInvPoints;
    const float fStepB = fCyclesB * fInvPoints;
    float fPos[kNumComponents];
    for (int i = 0; i < kNumComponents; ++i) {
        fPos[i] = mEnds[BeamParams::kEndStart][i] + fStep[i];
    }
    float fPhaseA = fStepA + fPhaseA0;
    float fPhaseB = fStepB + fPhaseB0;
    for (int nPoint = 1; nPoint < nPoints; ++nPoint) {
        const float fWaveA = FastSin(fPhaseA) * kFirstWaveScale;
        fPhaseA += fStepA;
        const float fWaveB = FastSin(fPhaseB) * kSecondWaveScale;
        fPhaseB += fStepB;
        Vector3 point;
        point.x = (fShift[0] * fWaveB) + ((fLift[0] * fWaveA) + fPos[0]);
        point.y = (fShift[1] * fWaveB) + ((fLift[1] * fWaveA) + fPos[1]);
        point.z = (fShift[2] * fWaveB) + ((fLift[2] * fWaveA) + fPos[2]);
        mLine->SetPoint(nPoint, point);
        for (int i = 0; i < kNumComponents; ++i) {
            fPos[i] += fStep[i];
        }
    }

    const Color *pStart = mParams->GetColor(BeamParams::kEndStart, &color);
    const Color *pFinish = mParams->GetColor(BeamParams::kEndFinish, &color);
    const float fKeep = 1.0f - fT;
    const Color blended{(pFinish->r * fT) + (pStart->r * fKeep),
                        (pFinish->g * fT) + (pStart->g * fKeep),
                        (pFinish->b * fT) + (pStart->b * fKeep),
                        (pFinish->a * fT) + (pStart->a * fKeep)};
    mMat->SetAlpha(blended.a);
    mMat->SetAmbient(blended);
    mLine->SetPoint(0,
                    Vector3{mEnds[BeamParams::kEndStart][0],
                            mEnds[BeamParams::kEndStart][1],
                            mEnds[BeamParams::kEndStart][2],
                            mEnds[BeamParams::kEndStart][3]});
    mLine->UpdateWorldXfm(nullptr, 0);
    return false;
}

void Beam::SetEnd(int nEnd, const float *pPosition) {
    for (int i = 0; i < Rnd::kXfmRowFloatCount; ++i) {
        mEnds[nEnd][i] = pPosition[i];
    }
}
