#include "app/hudletterexit.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxutil.h"
#include "math/transform.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The tick of a banner that does not fly.
constexpr float kNoFlight = -1.0e9f;

// The fade of the pulse each poll.
constexpr float kPulseFade = 0.1f;

// The pulse SetTime() adds to its level.
constexpr float kPulseKick = 0.5f;

// The scale of the pulse is kPulseRest plus kPulseGain times the pulse.
constexpr float kPulseGain = 0.25f;
constexpr float kPulseRest = 0.75f;

// The value GfxManager::kPendingPointsLost has, which flies the letter of the second side.
constexpr int kResultLost = 2;

// The ticks over which the flight turns the flying text towards the identity.
constexpr float kTurnTicks = 250.0f;

// The index of the first letter path, whose length every flight uses.
constexpr int kFirstLetter = 0;

// The rows of a transform.
enum XfmRow {
    kRowX = 0,
    kRowY = 1,
    kRowZ = 2,
};

// The columns of a row of a transform.
enum XfmColumn {
    kColumnX = 0,
    kColumnY = 1,
    kColumnZ = 2,
    kColumnW = 3,
};

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

// The basis row of a transform with an index of XfmRow.
Vector3 &XfmRowAt(Transform &xfm, int nRow) {
    switch (nRow) {
    case kRowX:
        return xfm.mBasisX;
    case kRowY:
        return xfm.mBasisY;
    default:
        return xfm.mBasisZ;
    }
}

// A basis that scales by the same factor along x and z. Its fourth words are never written.
struct ScaleBasis {
    float mRows[HudLetterExit::kBasisRows][HudLetterExit::kRowFloats];
};

ScaleBasis MakeScaleBasis(float fScale) {
    ScaleBasis basis;
    basis.mRows[kRowX][kColumnX] = fScale;
    basis.mRows[kRowX][kColumnY] = 0.0f;
    basis.mRows[kRowX][kColumnZ] = 0.0f;
    basis.mRows[kRowY][kColumnX] = 0.0f;
    basis.mRows[kRowY][kColumnY] = 1.0f;
    basis.mRows[kRowY][kColumnZ] = 0.0f;
    basis.mRows[kRowZ][kColumnX] = 0.0f;
    basis.mRows[kRowZ][kColumnY] = 0.0f;
    basis.mRows[kRowZ][kColumnZ] = fScale;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    return basis;
#pragma GCC diagnostic pop
}

// Copy one row of a basis, including its unset fourth word.
void CopyBasisRow(RndTransformable *pTrans, const ScaleBasis &basis, int nRow) {
    Vector3 &row = XfmRowAt(pTrans->mLocalXfm, nRow);
    row.x = basis.mRows[nRow][kColumnX];
    row.y = basis.mRows[nRow][kColumnY];
    row.z = basis.mRows[nRow][kColumnZ];
    row.w = basis.mRows[nRow][kColumnW];
}

// Write the three basis rows of a local transform and mark it dirty.
void SetBasis(RndTransformable *pTrans, const ScaleBasis &basis) {
    CopyBasisRow(pTrans, basis, kRowX);
    CopyBasisRow(pTrans, basis, kRowY);
    pTrans->mDirty = 1; // Yes, the binary marks the transform before the last row is written.
    CopyBasisRow(pTrans, basis, kRowZ);
}

// The scale of an identity basis.
constexpr float kIdentityScale = 1.0f;

} // namespace

const char *HudLetterExit::sLetters = "AMPLITUDE";

HudLetterExit::HudLetterExit() {
    mFlight = nullptr;
    mFlipped = 0;
    mSecondSidePlayer = -1;
    mFlightStart = kNoFlight;
    mView = FindObject<RndView>(FormatString("%s letter_exit.view", Overlay::sHudPrefix));
    for (int i = 0; i < kNumSides; ++i) {
        int nSide = 0;
        if (TheGameDb->GetPlayerSlot(i) != 0) {
            mSecondSidePlayer = i;
            nSide = 1;
        }
        mTexts[i] =
            FindObject<RndText>(FormatString("%s letter_exit%d.txt", Overlay::sHudPrefix, nSide));
        mAnims[i] = FindObject<RndTransAnim>(
            FormatString("%s letter_exit%d.tnm", Overlay::sHudPrefix, nSide));
    }
    for (int i = 0; i < kNumLetters; ++i) {
        mLetterPaths[i] = FindObject<RndTransAnim>(
            FormatString("%s letter_exit %cr.tnm", Overlay::sHudPrefix, sLetters[i]));
    }
    mOrigin = static_cast<RndTransformable *>(mView)->mLocalXfm.mTranslation;
    mFlightLength = mLetterPaths[kFirstLetter]->mFramesOwner->mTransKeys.back().frame;
    Reset();
}

void HudLetterExit::Reset() {
    static_cast<RndDrawable *>(mView)->SetShowing(false);
    mPulse = 0.0f;
    mFlightStart = kNoFlight;
    mPendingPlayer = 0;
    mPulseFloor = 0.0f;
    mLetters[1] = 0;
    mLetters[0] = 0;
    mFlight = nullptr;
    mRevealPending = 0;
    mPendingPoints = 0;
}

void HudLetterExit::Poll() {
    const float fTick = TheGameDb->mSongTick;
    ScaleByPulse(mView);
    if (mFlightStart == kNoFlight) {
        mAnims[0]->SetFrame(fTick);
        mAnims[1]->SetFrame(fTick);
    } else {
        float fTicks = fTick - mFlightStart;
        if (Overlay::sDuelPointsMoveTicks < fTicks) {
            fTicks = Overlay::sDuelPointsMoveTicks;
        } else if (fTicks < 0.0f) {
            fTicks = 0.0f;
        }
        // Yes, the binary leaves the fourth words of the identity basis unset.
        float basis[kBasisRows][kRowFloats];
        basis[kRowX][kColumnX] = 1.0f;
        basis[kRowX][kColumnY] = 0.0f;
        basis[kRowX][kColumnZ] = 0.0f;
        basis[kRowY][kColumnX] = 0.0f;
        basis[kRowY][kColumnY] = 1.0f;
        basis[kRowY][kColumnZ] = 0.0f;
        basis[kRowZ][kColumnX] = 0.0f;
        basis[kRowZ][kColumnY] = 0.0f;
        basis[kRowZ][kColumnZ] = 1.0f;
        if (fTicks < kTurnTicks) {
            // Yes, the binary discards this call's result.
            InterpBasis(mFlyBasis[0], basis[0], basis[0], fTicks / kTurnTicks);
        }
        const float fFrame =
            mFlight->FilterFrame(fTicks * Overlay::sDuelPointsMoveRate * mFlightLength);
        Transform xfm;
        mFlight->MakeTransform(fFrame, xfm, 1);
        if (mFlipped != 0) {
            xfm.mTranslation.x = -xfm.mTranslation.x;
        }
        xfm.mTranslation.x += mOrigin.x;
        xfm.mTranslation.y += mOrigin.y;
        xfm.mTranslation.z += mOrigin.z;
        RndTransformable *pViewTrans = mView;
        pViewTrans->mLocalXfm.mBasisX = xfm.mBasisX;
        pViewTrans->mLocalXfm.mBasisY = xfm.mBasisY;
        pViewTrans->mLocalXfm.mBasisZ = xfm.mBasisZ;
        // Yes, the binary marks the transform before the last row is written.
        pViewTrans->mDirty = 1;
        pViewTrans->mLocalXfm.mTranslation = xfm.mTranslation;
        if (fTicks == Overlay::sDuelPointsMoveTicks) {
            if (mRevealPending != 0) {
                const int nPoints = mPendingPoints;
                const int nPlayer = mPendingPlayer;
                Reset();
                ShowReveal(nPlayer, nPoints, nullptr);
            } else {
                Reset();
            }
        }
    }
    mPulse -= kPulseFade;
    if (mPulse < mPulseFloor) {
        mPulse = mPulseFloor;
    }
}

void HudLetterExit::ShowReveal(int nPlayer, int nPoints, [[maybe_unused]] const char *pszText) {
    if (mFlightStart != kNoFlight) {
        mPendingPoints = nPoints;
        mRevealPending = 1;
        mPendingPlayer = nPlayer;
        return;
    }
    int nLetter = nPoints - 1;
    if (nLetter < 0) {
        nLetter = 0;
    } else if (nLetter >= kNumLetters) {
        nLetter = kNumLetters - 1;
    }
    const char szLetter[] = {sLetters[nLetter], '\0'};
    mTexts[nPlayer]->SetText(szLetter);
    mLetters[nPlayer] = nLetter;
    mPulseFloor = 0.0f;
    static_cast<RndDrawable *>(mView)->SetShowing(true);
    mTexts[0]->SetShowing(true);
    mTexts[1]->SetShowing(true);
    RndTransformable *pViewTrans = mView;
    SetBasis(pViewTrans, MakeScaleBasis(kIdentityScale));
    pViewTrans->mDirty = 1;
    pViewTrans->mLocalXfm.mTranslation = mOrigin;
    mRevealPending = 0;
}

void HudLetterExit::ScaleByPulse(RndTransformable *pTrans) {
    const ScaleBasis basis =
        MakeScaleBasis(Overlay::sPointsScale * (mPulse * kPulseGain + kPulseRest));
    pTrans->mDirty = 1;
    CopyBasisRow(pTrans, basis, kRowX);
    CopyBasisRow(pTrans, basis, kRowY);
    CopyBasisRow(pTrans, basis, kRowZ);
}

void HudLetterExit::SetTime(float fLevel) {
    mPulseFloor = fLevel;
    mPulse = fLevel + kPulseKick;
}

void HudLetterExit::Fly(int nResult) {
    const int nSide = nResult == kResultLost;
    mTexts[nSide]->SetShowing(true);
    mTexts[nSide ^ 1]->SetShowing(false);
    RndTransformable *pTextTrans = mTexts[nSide];
    for (int i = kRowX; i <= kRowZ; ++i) {
        const Vector3 &row = XfmRowAt(pTextTrans->mLocalXfm, i);
        mFlyBasis[i][kColumnX] = row.x;
        mFlyBasis[i][kColumnY] = row.y;
        mFlyBasis[i][kColumnZ] = row.z;
        mFlyBasis[i][kColumnW] = row.w;
    }
    mFlightStart = TheGameDb->mSongTick;
    mFlipped = (nSide ^ mSecondSidePlayer) != 0;
    mFlight = mLetterPaths[mLetters[nSide]];
    SetBasis(pTextTrans, MakeScaleBasis(kIdentityScale));
}
