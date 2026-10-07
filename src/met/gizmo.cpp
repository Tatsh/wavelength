#include "met/gizmo.h"

#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "math/transformops.h"
#include "rnd/manager.h"
#include "rnd/transformable.h"

namespace {

// Rows of a transform, three of the basis and the translation.
constexpr int kTranslationRow = 3;
constexpr int kX = 0;
constexpr int kY = 1;
constexpr int kZ = 2;
constexpr int kW = 3;

// The curve the constructor builds before Init()'s frames apply.
constexpr float kDefaultSeverity = 10.0f;

// The fractions at which a move stands at its start and at its end.
constexpr float kMoveStart = 0.0f;
constexpr float kMoveEnd = 1.0f;

void CopyToRow(const Vector3 &vector, float *pRow) {
    pRow[kX] = vector.x;
    pRow[kY] = vector.y;
    pRow[kZ] = vector.z;
    pRow[kW] = vector.w;
}

void CopyFromRow(const float *pRow, Vector3 &vector) {
    vector.x = pRow[kX];
    vector.y = pRow[kY];
    vector.z = pRow[kZ];
    vector.w = pRow[kW];
}

} // namespace

float Gizmo::sBeginFrame = 0.0f;
float Gizmo::sAnimStart = 0.0f;
float Gizmo::sAnimEnd = 960.0f;
float Gizmo::sSeverity = 1.0f;

Gizmo::Gizmo(int nUnused)
    : mUnused(nUnused), mInterp(0.0f, 0.0f, 0.0f, kMoveEnd, kDefaultSeverity) {
    mView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("gizmo.view"));
    mView->SetShowing(true);
    mMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("gizmo.mesh"));
    mMovie = dynamic_cast<Rnd::Movie *>(Rnd::TheManager.Find("gizmo movie.mov"));
    mView->RemoveAnim(mMovie);
    mMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("freqscreen.mat"));
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
    return mView->GetShowing() != 0;
}

void Gizmo::SetShowAvatar(bool bShowAvatar) {
    Rnd::Mat::BlendMode blend;
    if (bShowAvatar) {
        mMat->mStages.front().SetTex(g_pAvatarTex);
        blend = Rnd::Mat::kBlendModeSrcAlpha;
    } else {
        mMat->mStages.front().SetTex(mMovie->mTex);
        blend = Rnd::Mat::kBlendModeAdd;
    }
    mMat->mBlend = blend;
}

bool Gizmo::IsShowingAvatar() const {
    return mMat->mBlend == Rnd::Mat::kBlendModeSrcAlpha;
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

    alignas(16) float xfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    if (mMoveStart == 0.0f) {
        Rnd::MakeRotMatrix(mRot, xfm[0]);
        CopyToRow(mPos, xfm[kTranslationRow]);
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
                CopyToRow(mFromPos, xfm[kTranslationRow]);
            } else if (fT == kMoveEnd) {
                CopyToRow(mPos, xfm[kTranslationRow]);
            } else {
                const float fFrom = kMoveEnd - fT;
                Vector3 position = mPos;
                position.x = mPos.x * fT + mFromPos.x * fFrom;
                position.y = mPos.y * fT + mFromPos.y * fFrom;
                position.z = mPos.z * fT + mFromPos.z * fFrom;
                CopyToRow(position, xfm[kTranslationRow]);
            }
            alignas(16) Quat rot;
            QuatSlerp(mFromRot, mRot, rot, fT);
            Rnd::MakeRotMatrix(rot, xfm[0]);
            if (sAnimEnd <= fFrame) {
                mMoveStart = 0.0f;
            }
        }
    }

    if (!IsShowingAvatar()) {
        mMovie->SetFrame(TheGameDb->mSongTick);
    }
    mView->SetFrame(fTime);
    XfmConcat(mMesh->mLocalXfm[0], xfm[0], xfm[0]);
    for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
        for (int j = 0; j < Rnd::kXfmRowFloatCount; ++j) {
            mMesh->mLocalXfm[i][j] = xfm[i][j];
        }
    }
    mMesh->mDirty = 1;
    mView->UpdateWorldXfm(nullptr, 0);
}

Transform Gizmo::WorldXfm() const {
    Transform xfm;
    CopyFromRow(mMesh->mWorldXfm[0], xfm.mBasisX);
    CopyFromRow(mMesh->mWorldXfm[1], xfm.mBasisY);
    CopyFromRow(mMesh->mWorldXfm[2], xfm.mBasisZ);
    CopyFromRow(mMesh->mWorldXfm[kTranslationRow], xfm.mTranslation);
    return xfm;
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
