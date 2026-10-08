#include "app/hudpoints.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "math/color.h"
#include "os/string.h"
#include "rnd/font.h"
#include "rnd/manager.h"

namespace {

// The rows of a transform: three of the basis, then the translation.
enum XfmRow { kXfmRowBasisX = 0, kXfmRowBasisY = 1, kXfmRowBasisZ = 2 };

// The speed of the exit animation, in frames per unit of time.
constexpr float kExitSpeed = 0.1f;

// The head-up display prefix of an online game.
constexpr char kOnlineHudPrefix[] = "HUDn";

// The glow and the swell shrink by these steps at every poll.
constexpr float kGlowStep = 0.2f;
constexpr float kSwellStep = 0.1f;

// The part of the size of the points the swell scales, and the part that stays.
constexpr float kSwellPart = 0.25f;
constexpr float kRestPart = 0.75f;

// The swell a flash starts at above its minimum.
constexpr float kFlashSwell = 0.5f;

// The exit animation of captured points starts at frame 0, and of lost points at frame 200. A
// duel delays its captured points by 200 frames too.
constexpr float kCapturedExitStart = 0.0f;
constexpr float kLostExitStart = 200.0f;
constexpr float kDuelLostDelay = 200.0f;
constexpr float kExitLength = 100.0f;

// The number of rows of the basis of a transform.
constexpr int kBasisRowCount = 3;

// A basis that scales by the same factor along x and z. Its fourth words are never written.
struct ScaleBasis {
    float mRows[kBasisRowCount][Rnd::kXfmRowFloatCount];
};

ScaleBasis MakeScaleBasis(float fScale) {
    ScaleBasis basis;
    basis.mRows[kXfmRowBasisX][0] = fScale;
    basis.mRows[kXfmRowBasisX][1] = 0.0f;
    basis.mRows[kXfmRowBasisX][2] = 0.0f;
    basis.mRows[kXfmRowBasisY][0] = 0.0f;
    basis.mRows[kXfmRowBasisY][1] = 1.0f;
    basis.mRows[kXfmRowBasisY][2] = 0.0f;
    basis.mRows[kXfmRowBasisZ][0] = 0.0f;
    basis.mRows[kXfmRowBasisZ][1] = 0.0f;
    basis.mRows[kXfmRowBasisZ][2] = fScale;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    return basis;
#pragma GCC diagnostic pop
}

// Copy one row of a basis, including its unset fourth word.
void CopyRow(float (&destination)[Rnd::kXfmRowFloatCount], const ScaleBasis &basis, int nRow) {
    for (int i = 0; i < Rnd::kXfmRowFloatCount; ++i) {
        destination[i] = basis.mRows[nRow][i];
    }
}

void SetScale(Rnd::Transformable *pTrans, float fScale) {
    const ScaleBasis basis = MakeScaleBasis(fScale);
    CopyRow(pTrans->mLocalXfm[kXfmRowBasisX], basis, kXfmRowBasisX);
    CopyRow(pTrans->mLocalXfm[kXfmRowBasisY], basis, kXfmRowBasisY);
    pTrans->mDirty = 1; // Yes, the binary marks the transform before the last row is written.
    CopyRow(pTrans->mLocalXfm[kXfmRowBasisZ], basis, kXfmRowBasisZ);
}

} // namespace

HudPoints::HudPoints(int nIndex)
    : mEnabled(1), mExit(kExitRestFrame, nullptr, nullptr, kExitSpeed) {
    const char *pszHud = Overlay::sHudPrefix;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        nIndex = 0;
        pszHud = kOnlineHudPrefix;
    }
    mExitBlur = dynamic_cast<Rnd::Blur *>(
        Rnd::TheManager.Find(FormatString("%s pts_exit%d.blur", pszHud, nIndex)));
    mExitView = dynamic_cast<Rnd::View *>(
        Rnd::TheManager.Find(FormatString("%s pts_exit%d.view", pszHud, nIndex)));
    auto *pExitText = dynamic_cast<Rnd::Text *>(
        Rnd::TheManager.Find(FormatString("%s pts_exit%d.txt", pszHud, nIndex)));
    mExitText = pExitText;
    SetScale(pExitText, Overlay::sPointsScale);
    mExit.SetAnim(mExitView);
    mPointsText = dynamic_cast<Rnd::Text *>(
        Rnd::TheManager.Find(FormatString("%s pts%d.txt", pszHud, nIndex)));
    mMultText = dynamic_cast<Rnd::Text *>(
        Rnd::TheManager.Find(FormatString("%s ptsmult%d.txt", pszHud, nIndex)));
    if (mPointsText->mFont != nullptr) {
        mPointsMat = mPointsText->mFont->mMat;
    }
    if (mMultText->mFont != nullptr) {
        mMultMat = mMultText->mFont->mMat;
    }
    mColdMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD ptstmp.mat"));
    mHotMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD ptstmphot.mat"));
    Reset();
}

void HudPoints::Reset() {
    mPoints = 0;
    mHot = 0;
    mPending = 0;
    mGlow = 0.0f;
    mMinSwell = 0.0f;
    mSwell = 0.0f;
    mMultiplier = 1;
    mExit.Jump(kExitRestFrame, kExitRestFrame);
    mPointsText->SetShowing(false);
    mMultText->SetShowing(false);
    static_cast<Rnd::Drawable *>(mExitView)->SetShowing(false);
}

void HudPoints::SetEnabled(bool bEnabled) {
    mEnabled = bEnabled;
    Reset();
}

void HudPoints::Poll([[maybe_unused]] float fUnused, float fDelta) {
    if (mEnabled == 0) {
        return;
    }
    mExit.Update(fDelta, false);
    static_cast<Rnd::Drawable *>(mExitView)->SetShowing(mExit.mValue != mExit.mInterp->mY1);
    ScaleText(mPointsText);
    const float fGlow = mGlow * 0.5f + 0.5f;
    mMultMat->SetAmbient(Color{fGlow, fGlow, fGlow, 1.0f});
    mPointsMat->SetAmbient((mHot != 0 ? mHotMat : mColdMat)->mAmbient);
    mGlow -= kGlowStep;
    if (mGlow < 0.0f) {
        mGlow = 0.0f;
    }
    mSwell -= kSwellStep;
    if (mSwell < mMinSwell) {
        mSwell = mMinSwell;
    }
}

void HudPoints::ScaleText(Rnd::Transformable *pText) {
    const ScaleBasis basis =
        MakeScaleBasis(Overlay::sPointsScale * (mSwell * kSwellPart + kRestPart));
    pText->mDirty = 1;
    CopyRow(pText->mLocalXfm[kXfmRowBasisX], basis, kXfmRowBasisX);
    CopyRow(pText->mLocalXfm[kXfmRowBasisY], basis, kXfmRowBasisY);
    CopyRow(pText->mLocalXfm[kXfmRowBasisZ], basis, kXfmRowBasisZ);
}

void HudPoints::ShowPoints(int nPoints, const char *pszText) {
    mPoints = nPoints;
    mPointsText->SetText(pszText);
    mPointsText->SetShowing(mEnabled);
    mGlow = 0.0f;
    mPending = 1;
    mMinSwell = 0.0f;
}

void HudPoints::SetMultiplier(int nMultiplier, int bHot, const char *pszText) {
    mMultiplier = nMultiplier;
    mHot = bHot;
    if (pszText[0] == '\0') {
        mMultText->SetShowing(false);
        return;
    }
    mMultText->SetShowing(mEnabled);
    mMultText->SetText(pszText);
}

void HudPoints::HideMultiplier() {
    mMultText->SetShowing(false);
}

void HudPoints::Flash(float fMinSwell) {
    mMinSwell = fMinSwell;
    mGlow = 1.0f;
    mSwell = fMinSwell + kFlashSwell;
}

void HudPoints::End(int nResult) {
    float fDelay = kCapturedExitStart;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel && nResult == GfxManager::kPendingPointsLost) {
        fDelay = kDuelLostDelay;
        nResult = GfxManager::kPendingPointsCaptured;
    }
    if (nResult == GfxManager::kPendingPointsCaptured) {
        if (mPending == 0) {
            return;
        }
        mPointsText->SetShowing(false);
        mPending = 0;
        mGlow = 1.0f;
        mMinSwell = 0.0f;
        mExitText->SetText(FormatString("%d", mPoints * mMultiplier));
        ScaleText(mExitText);
        mExit.Jump(fDelay, fDelay + kExitLength);
    } else if (nResult == GfxManager::kPendingPointsLost) {
        if (mPending == 0) {
            return;
        }
        mPointsText->SetShowing(false);
        mPending = 0;
        mMinSwell = 0.0f;
        mExitText->SetText(FormatString("%d", mPoints));
        ScaleText(mExitText);
        mExit.Jump(kLostExitStart, kLostExitStart + kExitLength);
    } else {
        mPointsText->SetShowing(false);
        mPending = 0;
        mMinSwell = 0.0f;
        return;
    }
    if (mExitBlur != nullptr) {
        mExitBlur->mXfms.clear();
    }
}
