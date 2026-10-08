#include "app/hudletterexit.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxutil.h"
#include "math/transform.h"
#include "os/string.h"
#include "rnd/manager.h"

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
    kRowTranslation = 3,
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
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

Vector3 ReadRow(const float (&row)[Rnd::kXfmRowFloatCount]) {
    return Vector3{row[kColumnX], row[kColumnY], row[kColumnZ], row[kColumnW]};
}

void WriteRow(float (&row)[Rnd::kXfmRowFloatCount], const Vector3 &value) {
    row[kColumnX] = value.x;
    row[kColumnY] = value.y;
    row[kColumnZ] = value.z;
    row[kColumnW] = value.w;
}

// A basis that scales by the same factor along x and z. Its fourth words are never written.
struct ScaleBasis {
    float mRows[kRowZ + 1][Rnd::kXfmRowFloatCount];
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
void CopyBasisRow(Rnd::Transformable *pTrans, const ScaleBasis &basis, int nRow) {
    for (int i = kColumnX; i <= kColumnW; ++i) {
        pTrans->mLocalXfm[nRow][i] = basis.mRows[nRow][i];
    }
}

// Write the three basis rows of a local transform and mark it dirty.
void SetBasis(Rnd::Transformable *pTrans, const ScaleBasis &basis) {
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
    mView = FindObject<Rnd::View>(FormatString("%s letter_exit.view", Overlay::sHudPrefix));
    for (int i = 0; i < kNumSides; ++i) {
        int nSide = 0;
        if (TheGameDb->GetPlayerSlot(i) != 0) {
            mSecondSidePlayer = i;
            nSide = 1;
        }
        mTexts[i] =
            FindObject<Rnd::Text>(FormatString("%s letter_exit%d.txt", Overlay::sHudPrefix, nSide));
        mAnims[i] = FindObject<Rnd::TransAnim>(
            FormatString("%s letter_exit%d.tnm", Overlay::sHudPrefix, nSide));
    }
    for (int i = 0; i < kNumLetters; ++i) {
        mLetterPaths[i] = FindObject<Rnd::TransAnim>(
            FormatString("%s letter_exit %cr.tnm", Overlay::sHudPrefix, sLetters[i]));
    }
    mOrigin = ReadRow(static_cast<Rnd::Transformable *>(mView)->mLocalXfm[kRowTranslation]);
    mFlightLength = mLetterPaths[kFirstLetter]->mFramesOwner->mTransKeys.back().mFrame;
    Reset();
}

void HudLetterExit::Reset() {
    static_cast<Rnd::Drawable *>(mView)->SetShowing(false);
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
        float basis[3][Rnd::kXfmRowFloatCount];
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
        float afXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
        mFlight->EvalFrame(fFrame, &afXfm[0][0], 1);
        if (mFlipped != 0) {
            afXfm[kRowTranslation][kColumnX] = -afXfm[kRowTranslation][kColumnX];
        }
        afXfm[kRowTranslation][kColumnX] += mOrigin.x;
        afXfm[kRowTranslation][kColumnY] += mOrigin.y;
        afXfm[kRowTranslation][kColumnZ] += mOrigin.z;
        Rnd::Transformable *pViewTrans = mView;
        for (int i = kRowX; i <= kRowZ; ++i) {
            WriteRow(pViewTrans->mLocalXfm[i], ReadRow(afXfm[i]));
        }
        // Yes, the binary marks the transform before the last row is written.
        pViewTrans->mDirty = 1;
        WriteRow(pViewTrans->mLocalXfm[kRowTranslation], ReadRow(afXfm[kRowTranslation]));
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
    static_cast<Rnd::Drawable *>(mView)->SetShowing(true);
    mTexts[0]->SetShowing(true);
    mTexts[1]->SetShowing(true);
    Rnd::Transformable *pViewTrans = mView;
    SetBasis(pViewTrans, MakeScaleBasis(kIdentityScale));
    pViewTrans->mDirty = 1;
    WriteRow(pViewTrans->mLocalXfm[kRowTranslation], mOrigin);
    mRevealPending = 0;
}

void HudLetterExit::ScaleByPulse(Rnd::Transformable *pTrans) {
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
    Rnd::Transformable *pTextTrans = mTexts[nSide];
    for (int i = kRowX; i <= kRowZ; ++i) {
        for (int j = kColumnX; j <= kColumnW; ++j) {
            mFlyBasis[i][j] = pTextTrans->mLocalXfm[i][j];
        }
    }
    mFlightStart = TheGameDb->mSongTick;
    mFlipped = (nSide ^ mSecondSidePlayer) != 0;
    mFlight = mLetterPaths[mLetters[nSide]];
    SetBasis(pTextTrans, MakeScaleBasis(kIdentityScale));
}
