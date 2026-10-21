#include "gfx/tnlsabretrail.h"

#include <cmath>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "math/rand.h"
#include "math/sine.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmesh.h"

namespace {

// `sample_envelope_frequency` is in turns per bar of 1920 ticks.
constexpr float kAnglePerTickPerTurn = 6.2831855f / 1920.0f;

// A random sweep of a sample covers this share of mSweep, at the least and at the most.
constexpr float kWobbleLow = 0.3f;
constexpr float kWobbleHigh = 0.6f;

// The points of the needle, at its base and at its tip.
constexpr int kNeedleBase = 0;
constexpr int kNeedleTip = 1;

// The colours of the points at the ends and in the middle of the stick.
constexpr int kColorLow = 0;
constexpr int kColorMiddle = 1;
constexpr int kColorHigh = 2;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

// Blend two colours, all four components.
inline Color Lerp(const Color &from, const Color &to, float flT) {
    const float flU = 1.0f - flT;
    return Color{(to.r * flT) + (from.r * flU),
                 (to.g * flT) + (from.g * flU),
                 (to.b * flT) + (from.b * flU),
                 (to.a * flT) + (from.a * flU)};
}

} // namespace

float TnlSabreTrail::sSampleFrequency = 6.2831855f;
float TnlSabreTrail::sSampleDuration = 400.0f;
Vector2 TnlSabreTrail::sSampleSweepTicks{100.0f, 100.0f};

TnlSabreTrail::TnlSabreTrail(int nPlayer, Ship *pShip)
    : mSweep(0.0f), mNeedleLength(0.0f), mRibbon(nullptr), mNeedle(0.0f, 0.0f, 0.0f, 1.0f),
      mState(kStateHidden), mRestart(0), mSweeping(0), mNewSegment(0), mSampling(0),
      mStopPending(0), mNumPoints(0), mHead(kNoHead) {
    mRibbon = pShip->GetScratchRibbon();
    RndMesh *pMesh = FindObject<RndMesh>(FormatString("[%s_mesh]", mRibbon->GetName()));
    if (pMesh != nullptr) {
        pMesh->SetZMode(RndMesh::kZModeDisable);
        pMesh->SetZFunc(RndMesh::kZFuncAlways);
    }
    Vector3 &translation = mRibbon->GetLocalXfm().mTranslation;
    mRibbonOffset = translation;
    translation.x = 0.0f;
    translation.y = 0.0f;
    translation.z = 0.0f;
    mRibbon->SetDirty(1);
    mRibbon->UpdateWorldXfm(nullptr, 0);
    mColor = *TheGfxManager.GetPlayerColor(nPlayer);
    for (int i = 0; i < 3; ++i) {
        mPointColors[i] = mRibbon->GetPoints()[i].GetColor();
    }
    mColor.a = mPointColors[kColorMiddle].a;
    mNumPoints = static_cast<int>(mRibbon->GetPoints().size());
}

TnlSabreTrail::~TnlSabreTrail() = default;

void TnlSabreTrail::LoadConfig(DataArray *pConfig, DataArray *pDefaults) {
    DataArray *pSection;
    FindConfigArray(pConfig, pDefaults, "scratch_ribbon", &pSection, true);
    float flWidth;
    pSection->FindFloat("width", &flWidth, true);
    mRibbon->SetWidth(flWidth * TheGfxManager.GetScrollSpeed());
    pSection->FindFloat("sweep", &mSweep, true);
    pSection->FindFloat("sample_envelope_frequency", &sSampleFrequency, true);
    sSampleFrequency *= kAnglePerTickPerTurn;
    pSection->FindVector("sample_sweep_duration", &sSampleSweepTicks, true);
    pSection->FindFloat("sample_duration", &sSampleDuration, true);
    pSection->FindFloat("needle_length", &mNeedleLength, true);
    // Yes, the binary scales the sweep and leaves the needle as configured.
    mSweep *= TheGfxManager.GetScrollSpeed();
    int nSegments;
    pSection->FindInt("segments", &nSegments, true);
    mNumPoints = nSegments * 2;
    mRibbon->SetNumPoints(mNumPoints);
    for (int i = 0; i < mNumPoints; ++i) {
        mRibbon->SetPointColor(i, mPointColors[kColorMiddle]);
        mRibbon->SetPoint(i, Vector3{0.0f, 0.0f, 0.0f});
    }
    mRibbon->SetShowing(1);
    if ((mNumPoints - 2) < mHead) {
        mHead = kFirstSegmentPoint;
    }
}

void TnlSabreTrail::AddSegment(const Vector3 *pPos) {
    const int nOld = mHead;
    if (nOld < kFirstSegmentPoint) {
        mRestart = 1;
        mHead = kFirstSegmentPoint;
        return;
    }
    mHead = nOld + 2;
    if (mHead >= mNumPoints) {
        mHead = kFirstSegmentPoint;
    }
    if (pPos != nullptr) {
        mRibbon->SetPoint(nOld, *pPos);
        mRibbon->SetPoint(mHead, *pPos);
        mRibbon->SetPoint(mHead + 1, *pPos);
        return;
    }
    mRibbon->SetPoint(mHead, mRibbon->GetPoints()[nOld].GetPoint());
    mRibbon->SetPoint(mHead + 1, mRibbon->GetPoints()[nOld].GetPoint());
}

void TnlSabreTrail::Start() {
    if (mState != kStateDrawing) {
        mSweeping = 0;
        mSampling = 0;
        mRestart = 1;
        AddSegment(nullptr);
        mState = kStateDrawing;
    }
    mStopPending = 0;
}

void TnlSabreTrail::Stop(bool bForce) {
    if (mState != kStateDrawing) {
        return;
    }
    if (mSampling && !bForce) {
        mStopPending = 1;
        return;
    }
    mState = kStateShown;
    mRibbon->SetPoint(kNeedleTip, mRibbon->GetPoints()[kNeedleBase].GetPoint());
    mNeedle.Reset(0.0f, 0.0f, 0.0f, 1.0f);
}

void TnlSabreTrail::Clear() {
    for (int i = 0; i < mNumPoints; ++i) {
        mRibbon->SetPoint(i, Vector3{0.0f, 0.0f, 0.0f});
    }
}

void TnlSabreTrail::SetShowing(bool bShowing) {
    if (bShowing == (mState != kStateHidden)) {
        return;
    }
    mHead = kNoHead;
    if (bShowing) {
        mState = kStateShown;
        mRibbon->SetShowing(1);
    } else {
        mState = kStateHidden;
        mRibbon->SetShowing(0);
    }
}

void TnlSabreTrail::Scratch(float flTick, float flDuration) {
    if (mSampling) {
        return;
    }
    const float flFrom = mNeedle.Eval(flTick);
    const float flTo = SweepTarget(mNeedle.GetY1(), mSweep);
    mNeedle.Reset(flFrom, flTo, TheGameDb->GetSongTick(), flTick + flDuration);
    if (mState == kStateDrawing) {
        mSweeping = 1;
        mNewSegment = 1;
    }
}

void TnlSabreTrail::StartSample() {
    mSampleEnd = TheGameDb->GetSongTick() + sSampleDuration;
    if (mSampling) {
        return;
    }
    const float flTick = TheGameDb->GetSongTick();
    const float flFrom = mNeedle.Eval(flTick);
    const float flTo = Wobble(mNeedle.GetY1(), mSweep);
    const float flTicks = RandomFloat(sSampleSweepTicks.x, sSampleSweepTicks.y);
    mNeedle.Reset(flFrom, flTo, flTick, flTick + flTicks);
    mSampling = 1;
    mNewSegment = 1;
    mSweeping = 1;
    if (mState != kStateDrawing) {
        mStopPending = 1;
        mRestart = 1;
        AddSegment(nullptr);
        mState = kStateDrawing;
    }
}

float TnlSabreTrail::UpdateSample() {
    const float flTick = TheGameDb->GetSongTick();
    if (mSampling) {
        if (mSampleEnd <= flTick) {
            mSampling = 0;
            if (mStopPending) {
                Stop(true);
            }
        } else if (mNeedle.GetX1() <= flTick) {
            const float flFrom = mNeedle.GetY1();
            const float flTo = Wobble(flFrom, mSweep);
            const float flTicks = RandomFloat(sSampleSweepTicks.x, sSampleSweepTicks.y);
            mNeedle.Reset(flFrom, flTo, flTick, flTick + flTicks);
            mSweeping = 1;
            mNewSegment = 1;
        }
    }
    return mNeedle.Eval(flTick);
}

void TnlSabreTrail::Update(float flBlend, const Transform &xfm, const Vector3 *) {
    if (mState != kStateDrawing) {
        return;
    }
    const float flTick = TheGameDb->GetSongTick();
    Color color;
    if (flBlend <= 0.0f) {
        color = Lerp(mPointColors[kColorLow], mPointColors[kColorMiddle], flBlend + 1.0f);
    } else {
        color = Lerp(mPointColors[kColorMiddle], mPointColors[kColorHigh], flBlend);
    }
    Vector3 base;
    MultiplyPoint(xfm, mRibbonOffset, base);
    const float flNeedle = -mNeedleLength;
    const Vector3 tip{(xfm.mBasisY.x * flNeedle) + base.x,
                      (xfm.mBasisY.y * flNeedle) + base.y,
                      (xfm.mBasisY.z * flNeedle) + base.z};
    if (mNewSegment && !mRestart) {
        AddSegment(&tip);
        mNewSegment = 0;
    }
    if (mHead < 0) {
        return;
    }
    if (mSweeping) {
        if (mNeedle.GetX1() <= flTick) {
            mSweeping = 0;
            mNewSegment = 1;
        } else if (!(flTick <= mNeedle.GetX0())) {
            (void)mNeedle.Interp(flTick); // Yes, the binary discards this call's result.
        }
    }
    mRibbon->SetPointColor(kNeedleBase, color);
    mRibbon->SetPointColor(kNeedleTip, color);
    mRibbon->SetPoint(kNeedleBase, base);
    mRibbon->SetPoint(kNeedleTip, tip);
    mRibbon->SetPoint(mHead, tip);
    mRibbon->SetPointColor(mHead, color);
    mRibbon->SetPointColor(mHead + 1, color);
    if (mRestart) {
        mRestart = 0;
        mRibbon->SetPoint(mHead + 1, tip);
    }
}

float TnlSabreTrail::SweepTarget(float flFrom, float flSweep) {
    return (flFrom == flSweep) ? -flSweep : flSweep;
}

float TnlSabreTrail::Wobble(float flFrom, float flSweep) {
    float flEnvelope = std::fabs(FastSin(TheGameDb->GetSongTick() * sSampleFrequency));
    if (0.0f < flFrom) {
        flEnvelope = -flEnvelope;
    }
    return flEnvelope * RandomFloat(flSweep * kWobbleLow, flSweep * kWobbleHigh);
}
