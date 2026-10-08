#include "app/hudhilitebox.h"

#include "game/gamedb.h"
#include "math/sine.h"
#include "os/debug.h"
#include "rnd/blur.h"
#include "rnd/manager.h"
#include "rnd/meshvert.h"

namespace {

// The rows of a transform: three of the basis, then the translation.
enum XfmRow { kXfmRowBasisX = 0, kXfmRowBasisY = 1, kXfmRowBasisZ = 2, kXfmRowTranslation = 3 };

// The display coordinates of the unit square of the screen: x runs from -299 to 299, and y from
// 210 at the top down to -210.
constexpr float kScreenWidth = 598.0f;
constexpr float kScreenLeft = -299.0f;
constexpr float kScreenHeight = -420.0f;
constexpr float kScreenTop = 210.0f;

// The vertices whose difference is the size of a corner.
constexpr int kCornerFarVert = 12;
constexpr int kCornerNearVert = 9;

// The vertex of the box at each place, row by row from the top left.
constexpr unsigned char kBoxVerts[] = {13, 12, 14, 15, 9, 8, 10, 11, 3, 2, 5, 7, 0, 1, 4, 6};

// The size of the box after Reset().
constexpr float kResetBoxSize = 100.0f;

// The fade times of the box and the arrow, in ticks.
constexpr float kBoxFadeTicks = 480.0f;
constexpr float kArrowFadeTicks = 240.0f;

// The ease of the arrow's move.
constexpr float kArrowMoveSeverity = 5.0f;

constexpr float kPi = 3.1415927f;
constexpr float kHalfPi = 1.5707964f;
constexpr float kDegreesPerHalfTurn = 180.0f;

// Blend two places by the progress of a move, taking either end exactly.
Vector3 Blend(const Vector3 &from, const Vector3 &to, float fProgress) {
    if (fProgress == 0.0f) {
        return from;
    }
    if (fProgress == 1.0f) {
        return to;
    }
    const float fRest = 1.0f - fProgress;
    Vector3 blend = to;
    blend.x = to.x * fProgress + from.x * fRest;
    blend.y = to.y * fProgress + from.y * fRest;
    blend.z = to.z * fProgress + from.z * fRest;
    return blend;
}

// Place a vertex of a mesh on the plane of the display.
void SetVert(Rnd::Mesh *pMesh, int nVert, float fX, float fZ) {
    Vector3 &point = pMesh->mVertsOwner->mVerts[nVert].mPoint;
    point.x = fX;
    point.y = 0.0f;
    point.z = fZ;
}

void SetTranslation(Rnd::Transformable *pTrans, const Vector3 &position) {
    float (&translation)[Rnd::kXfmRowFloatCount] = pTrans->mLocalXfm[kXfmRowTranslation];
    translation[0] = position.x;
    translation[1] = position.y;
    translation[2] = position.z;
    translation[3] = position.w;
    pTrans->mDirty = 1;
}

} // namespace

HudHiliteBox::HudHiliteBox()
    : mBoxMove(1.0f, 1.0f, 0.0f, 1.0f), mBoxFade(0.0f, 0.0f, 0.0f, 1.0f),
      mArrowFade(0.0f, 0.0f, 0.0f, 1.0f), mArrowMove(1.0f, 1.0f, 0.0f, 1.0f, kArrowMoveSeverity) {
    mBox = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("HUD hilite_box.mesh"));
    mBoxMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD hilite_box.mat"));
    dynamic_cast<Rnd::Blur *>(Rnd::TheManager.Find("HUD hilite_box.blur"))->SetShowing(true);
    mArrow = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("HUD hilite arrow.view"));
    mArrowMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD hilite arrow.mat"));
    static_cast<Rnd::Drawable *>(mArrow)->SetShowing(false);
    mArrowMat->SetAlpha(0.0f);
    for (int i = 0; i < kNumBoxVerts; ++i) {
        mBoxVerts[i] = kBoxVerts[i];
    }
    const std::vector<Rnd::MeshVert> &verts = mBox->mVertsOwner->mVerts;
    mCornerSize.x = verts[kCornerFarVert].mPoint.x - verts[kCornerNearVert].mPoint.x;
    mCornerSize.y = verts[kCornerFarVert].mPoint.z - verts[kCornerNearVert].mPoint.z;
    Reset();
}

void HudHiliteBox::Reset() {
    mBox->SetShowing(false);
    static_cast<Rnd::Drawable *>(mArrow)->SetShowing(false);
    mBoxMove.Reset(1.0f, 1.0f, 0.0f, 1.0f);
    mBoxFade.Reset(0.0f, 0.0f, 0.0f, 1.0f);
    mArrowFade.Reset(0.0f, 0.0f, 0.0f, 1.0f);
    mArrowMove.Reset(1.0f, 1.0f, 0.0f, 1.0f);
    mToPos.x = 0.0f;
    mToSize.y = kResetBoxSize;
    mToSize.x = kResetBoxSize;
    mToPos.z = 0.0f;
    mToPos.y = 0.0f;
    mFromSize = mToSize;
    mFromPos = mToPos;
    SetTranslation(mBox, mToPos);
    mChanging = 0;
}

void HudHiliteBox::UnitToScreen(float *pfX0, float *pfY0, float *pfX1, float *pfY1) {
    *pfX0 = *pfX0 * kScreenWidth + kScreenLeft;
    *pfY0 = *pfY0 * kScreenHeight + kScreenTop;
    *pfX1 = *pfX1 * kScreenWidth + kScreenLeft;
    *pfY1 = *pfY1 * kScreenHeight + kScreenTop;
}

void HudHiliteBox::SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration) {
    UnitToScreen(&fX0, &fY0, &fX1, &fY1);
    const float fWidth = fX1 - fX0;
    const float fNow = TheGameDb->mSongTick;
    const float fHeight = fY0 - fY1;
    if (!(mCornerSize.x + mCornerSize.x < fWidth)) {
        DebugNotify("too narrow.\n");
    }
    if (!(mCornerSize.y + mCornerSize.y < fHeight)) {
        DebugNotify("too short.\n");
    }
    const float (&current)[Rnd::kXfmRowFloatCount] = mBox->mLocalXfm[kXfmRowTranslation];
    mFromPos.x = current[0];
    mFromPos.y = current[1];
    mFromPos.z = current[2];
    mFromPos.w = current[3];
    mToPos.x = (fX0 + fX1) * 0.5f;
    mToPos.z = (fY0 + fY1) * 0.5f;
    mToPos.y = 0.0f;
    const float fProgress = mBoxMove.Interpolator::Eval(fNow);
    mFromSize.x = (mToSize.x - mFromSize.x) * fProgress + mFromSize.x;
    mFromSize.y = (mToSize.y - mFromSize.y) * fProgress + mFromSize.y;
    mToSize.x = fWidth;
    mToSize.y = fHeight;
    mChanging |= kChangingBoxMove;
    mBoxMove.Reset(0.0f, 1.0f, fNow, fNow + fDuration);
}

void HudHiliteBox::ShowBox(bool bShow) {
    const float fAlpha = bShow ? 1.0f : 0.0f;
    if (fAlpha == mBoxFade.mY1) {
        return;
    }
    mChanging |= kChangingBoxFade;
    const float fNow = TheGameDb->mSongTick;
    mBoxFade.Reset(mBoxFade.Interpolator::Eval(fNow), fAlpha, fNow, fNow + kBoxFadeTicks);
}

void HudHiliteBox::SetArrowTarget(float fX, float fY, float fAngle, float fDuration) {
    const float fNow = TheGameDb->mSongTick;
    float fUnused = 0.0f;
    UnitToScreen(&fX, &fY, &fUnused, &fUnused);
    const float fProgress = mArrowMove.Interpolator::Eval(fNow);
    mFromArrowPos = Blend(mFromArrowPos, mToArrowPos, fProgress);
    mFromArrowAngle = (mToArrowAngle - mFromArrowAngle) * fProgress + mFromArrowAngle;
    mToArrowPos.x = fX;
    mToArrowPos.z = fY;
    mToArrowPos.y = 0.0f;
    mToArrowAngle = fAngle * kPi / kDegreesPerHalfTurn;
    // Yes, the binary starts the move one tick in the past.
    mArrowMove.Reset(0.0f, 1.0f, fNow - 1.0f, fNow + fDuration);
    mChanging |= kChangingArrowMove;
}

void HudHiliteBox::ShowArrow(bool bShow) {
    const float fAlpha = bShow ? 1.0f : 0.0f;
    if (fAlpha == mArrowFade.mY1) {
        return;
    }
    mChanging |= kChangingArrowFade;
    const float fNow = TheGameDb->mSongTick;
    mArrowFade.Reset(mArrowFade.Interpolator::Eval(fNow), fAlpha, fNow, fNow + kArrowFadeTicks);
}

void HudHiliteBox::Update(float fTick) {
    if (mChanging == 0) {
        return;
    }
    if ((mChanging & kChangingArrowFade) != 0) {
        const float fAlpha = mArrowFade.Interpolator::Eval(fTick);
        mArrowMat->SetAlpha(fAlpha);
        static_cast<Rnd::Drawable *>(mArrow)->SetShowing(fAlpha != 0.0f);
        if (mArrowFade.mX1 <= fTick) {
            mChanging &= ~kChangingArrowFade;
        }
    }
    if ((mChanging & kChangingArrowMove) != 0) {
        const float fProgress = mArrowMove.Interpolator::Eval(fTick);
        Rnd::Transformable *pArrow = mArrow;
        SetTranslation(pArrow, Blend(mFromArrowPos, mToArrowPos, fProgress));
        const float fAngle = (mToArrowAngle - mFromArrowAngle) * fProgress + mFromArrowAngle;
        const float fCos = SinApprox(fAngle + kHalfPi);
        const float fSin = SinApprox(fAngle);
        float (&xfm)[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount] = pArrow->mLocalXfm;
        xfm[kXfmRowBasisX][0] = fCos;
        xfm[kXfmRowBasisX][1] = 0.0f;
        xfm[kXfmRowBasisX][2] = -fSin;
        xfm[kXfmRowBasisY][0] = 0.0f;
        xfm[kXfmRowBasisY][1] = 1.0f;
        xfm[kXfmRowBasisY][2] = 0.0f;
        xfm[kXfmRowBasisZ][0] = fSin;
        xfm[kXfmRowBasisZ][1] = 0.0f;
        xfm[kXfmRowBasisZ][2] = fCos;
        pArrow->mDirty = 1;
        if (mArrowMove.mX1 <= fTick) {
            mChanging &= ~kChangingArrowMove;
        }
    }
    if ((mChanging & kChangingBoxFade) != 0) {
        const float fAlpha = mBoxFade.Interpolator::Eval(fTick);
        mBoxMat->SetAlpha(fAlpha);
        mBox->SetShowing(fAlpha != 0.0f);
        if (mBoxFade.mX1 <= fTick) {
            mChanging &= ~kChangingBoxFade;
        }
    }
    if ((mChanging & kChangingBoxMove) == 0) {
        return;
    }
    const float fProgress = mBoxMove.Interpolator::Eval(fTick);
    if (fProgress == 1.0f) {
        mChanging &= ~kChangingBoxMove;
    }
    SetTranslation(mBox, Blend(mFromPos, mToPos, fProgress));
    const float fHalfWidth = ((mToSize.x - mFromSize.x) * fProgress + mFromSize.x) * 0.5f;
    const float fHalfHeight = ((mToSize.y - mFromSize.y) * fProgress + mFromSize.y) * 0.5f;
    const float fCornerX = mCornerSize.x;
    const float fCornerZ = mCornerSize.y;
    const float afX[] = {-fHalfWidth, -fHalfWidth + fCornerX, fHalfWidth - fCornerX, fHalfWidth};
    const float afZ[] = {
        fHalfHeight, fHalfHeight - fCornerZ, -fHalfHeight + fCornerZ, -fHalfHeight};
    constexpr int kVertsPerRow = 4;
    for (int nRow = 0; nRow < kVertsPerRow; ++nRow) {
        for (int nColumn = 0; nColumn < kVertsPerRow; ++nColumn) {
            SetVert(mBox, mBoxVerts[nRow * kVertsPerRow + nColumn], afX[nColumn], afZ[nRow]);
        }
    }
    mBox->Sync();
}
