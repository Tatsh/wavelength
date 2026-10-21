#include "met/gizmo.h"

#include "game/avatar.h"
#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "math/transformops.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtransformable.h"

namespace {

// The curve the constructor builds before Init()'s frames apply.
constexpr float kDefaultSeverity = 10.0f;

// The fractions at which a move stands at its start and at its end.
constexpr float kMoveStart = 0.0f;
constexpr float kMoveEnd = 1.0f;

} // namespace

float Gizmo::sBeginFrame = 0.0f;
float Gizmo::sAnimStart = 0.0f;
float Gizmo::sAnimEnd = 960.0f;
float Gizmo::sSeverity = 1.0f;

Gizmo::Gizmo(int nUnused)
    : mUnused(nUnused), mInterp(0.0f, 0.0f, 0.0f, kMoveEnd, kDefaultSeverity) {
    mView = dynamic_cast<RndView *>(TheManager.Find("gizmo.view"));
    mView->SetShowing(true);
    mMesh = dynamic_cast<RndMesh *>(TheManager.Find("gizmo.mesh"));
    mMovie = dynamic_cast<RndMovie *>(TheManager.Find("gizmo movie.mov"));
    mView->RemoveAnim(mMovie);
    mMat = dynamic_cast<RndMat *>(TheManager.Find("freqscreen.mat"));
    SetShowAvatar(false);
    mMoveStart = 0.0f;
    mInterp.Reset(mMoveStart, kMoveEnd, sAnimStart, sAnimEnd, sSeverity);
}

Gizmo::~Gizmo() {
}

void Gizmo::Init(DataArray *pConfig) {
    DataArray *pScreen = pConfig->FindArray("freq_screen", true);
    pScreen->FindFloat("panel_gizmo_begin_frame", &sBeginFrame, false);
    pScreen->FindFloat("panel_gizmo_anim_start", &sAnimStart, false);
    pScreen->FindFloat("panel_gizmo_anim_end", &sAnimEnd, false);
    pScreen->FindFloat("panel_gizmo_interp_severity", &sSeverity, false);
}

void Gizmo::SetShowing(bool bShowing) {
    mView->SetShowing(bShowing);
}

bool Gizmo::IsShowing() const {
    return mView->mShowing != 0;
}

void Gizmo::SetShowAvatar(bool bShowAvatar) {
    int nBlend;
    if (bShowAvatar) {
        mMat->mStages.front().SetTex(Avatar::GetTargetTex());
        nBlend = RndMat::kBlendSrcAlpha;
    } else {
        mMat->mStages.front().SetTex(mMovie->mTex);
        nBlend = RndMat::kBlendAdd;
    }
    mMat->mBlend = nBlend;
}

bool Gizmo::IsShowingAvatar() const {
    return mMat->mBlend == RndMat::kBlendSrcAlpha;
}

void Gizmo::Draw() {
    if (IsShowingAvatar()) {
        TheGameDb->GetAvatar(0)->Render(nullptr);
    }
    mView->Draw();
}

void Gizmo::Poll(float fTime) {
    if (!IsShowing()) {
        return;
    }

    Transform xfm;
    if (mMoveStart == 0.0f) {
        Rnd::MakeRotMatrix(mRot, &xfm.mBasisX.x);
        xfm.mTranslation = mPos;
    } else {
        float fElapsed = fTime - mMoveStart;
        // Yes, before the begin frame the binary places the mesh with an unset transform.
        if (sBeginFrame <= fElapsed) {
            if (sAnimEnd < fElapsed) {
                fElapsed = sAnimEnd;
            }
            const float fFrame = sAnimStart + (fElapsed - sBeginFrame);
            const float fT = mInterp.ATanInterpolator::Interp(fFrame);
            if (fT == kMoveStart) {
                xfm.mTranslation = mFromPos;
            } else if (fT == kMoveEnd) {
                xfm.mTranslation = mPos;
            } else {
                const float fFrom = kMoveEnd - fT;
                Vector3 position = mPos;
                position.x = mPos.x * fT + mFromPos.x * fFrom;
                position.y = mPos.y * fT + mFromPos.y * fFrom;
                position.z = mPos.z * fT + mFromPos.z * fFrom;
                xfm.mTranslation = position;
            }
            alignas(16) Quat rot;
            QuatSlerp(mFromRot, mRot, rot, fT);
            Rnd::MakeRotMatrix(rot, &xfm.mBasisX.x);
            if (sAnimEnd <= fFrame) {
                mMoveStart = 0.0f;
            }
        }
    }

    if (!IsShowingAvatar()) {
        mMovie->SetFrame(TheGameDb->GetSongTick());
    }
    mView->SetFrame(fTime);
    sceVu0MulAffineMatrixXyz(&xfm.mBasisX.x, &xfm.mBasisX.x, &mMesh->mLocalXfm.mBasisX.x);
    mMesh->mLocalXfm = xfm;
    mMesh->mDirty = 1;
    mView->UpdateWorldXfm(nullptr, 0);
}

Transform Gizmo::WorldXfm() const {
    return mMesh->mWorldXfm;
}

void Gizmo::MoveTo(const Vector3 &position, const Quat &orientation, float fTime) {
    const Vector3 newPos = position;
    const Quat newRot = orientation;
    SetShowing(true);
    const Vector3 oldPos = mPos;
    const Quat oldRot = mRot;
    mMoveStart = fTime;
    mRot = newRot;
    mFromPos = oldPos;
    mFromRot = oldRot;
    mPos = newPos;
}

void Gizmo::SetTo(const Vector3 &position, const Quat &orientation) {
    mRot = orientation;
    mPos = position;
}
