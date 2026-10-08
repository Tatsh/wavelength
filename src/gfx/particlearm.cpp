#include "gfx/particlearm.h"

#include "gfx/camslide.h"
#include "math/transform.h"
#include "rnd/cam.h"

namespace {

// The components of a position.
constexpr int kNumComponents = 3;

// Read the four rows of a transform out of a row array.
inline Transform ToTransform(const float (*pRows)[Rnd::kXfmRowFloatCount]) {
    Transform xfm;
    Vector3 *rows[] = {&xfm.mBasisX, &xfm.mBasisY, &xfm.mBasisZ, &xfm.mTranslation};
    for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
        rows[i]->x = pRows[i][0];
        rows[i]->y = pRows[i][1];
        rows[i]->z = pRows[i][2];
        rows[i]->w = pRows[i][kVec3PaddingFloat];
    }
    return xfm;
}

// Invert a transform whose basis rows are orthogonal but need not be of unit length.
inline Transform InvertScaled(const Transform &xfm) {
    const Vector3 &x = xfm.mBasisX;
    const Vector3 &y = xfm.mBasisY;
    const Vector3 &z = xfm.mBasisZ;
    const float fInvX = 1.0f / ((x.x * x.x) + (x.y * x.y) + (x.z * x.z));
    const float fInvY = 1.0f / ((y.x * y.x) + (y.y * y.y) + (y.z * y.z));
    const float fInvZ = 1.0f / ((z.x * z.x) + (z.y * z.y) + (z.z * z.z));
    Transform inverse;
    inverse.mBasisX = Vector3{x.x * fInvX, y.x * fInvY, z.x * fInvZ};
    inverse.mBasisY = Vector3{x.y * fInvX, y.y * fInvY, z.y * fInvZ};
    inverse.mBasisZ = Vector3{x.z * fInvX, y.z * fInvY, z.z * fInvZ};
    const Vector3 back{-xfm.mTranslation.x, -xfm.mTranslation.y, -xfm.mTranslation.z};
    inverse.mTranslation = Vector3{
        (inverse.mBasisX.x * back.x) + (inverse.mBasisY.x * back.y) + (inverse.mBasisZ.x * back.z),
        (inverse.mBasisX.y * back.x) + (inverse.mBasisY.y * back.y) + (inverse.mBasisZ.y * back.z),
        (inverse.mBasisX.z * back.x) + (inverse.mBasisY.z * back.y) + (inverse.mBasisZ.z * back.z)};
    return inverse;
}

} // namespace

ParticleArm::~ParticleArm() = default;

void ParticleArm::Init(Rnd::ParticleSys *pSys, Rnd::Transformable *pTrans, float fScale) {
    mSys = pSys;
    mTrans = pTrans;
    pTrans->RemoveTrans(pSys);
    SetScale(fScale);
    mSys->UpdateWorldXfm(nullptr, 0);
}

void ParticleArm::SetScale(float fScale) {
    mInvScale = 1.0f / fScale;
    Scale(mSys, fScale);
}

void ParticleArm::Scale(Rnd::ParticleSys *pSys, float fScale) {
    pSys->mSizeLow *= fScale;
    pSys->mSizeHigh *= fScale;
    pSys->mSpeed.x *= fScale;
    pSys->mSpeed.y *= fScale;
    pSys->mPosLow.x *= fScale;
    pSys->mPosLow.y *= fScale;
    pSys->mPosLow.z *= fScale;
    pSys->mPosHigh.x *= fScale;
    pSys->mPosHigh.y *= fScale;
    pSys->mPosHigh.z *= fScale;
    pSys->mBubbleSize.x *= fScale;
    pSys->mBubbleSize.y *= fScale;
    for (int i = 0; i < kNumComponents; ++i) {
        pSys->mLocalXfm[Rnd::kXfmRowCount - 1][i] *= fScale;
    }
    pSys->mDirty = 1;
}

void ParticleArm::Draw() {
    Rnd::Cam *pCam = Rnd::Cam::sCurrent;
    if (pCam == nullptr) {
        return;
    }
    Transform arm = ToTransform(mTrans->mWorldXfm);
    Vector3 *basis[] = {&arm.mBasisX, &arm.mBasisY, &arm.mBasisZ};
    for (Vector3 *pRow : basis) {
        pRow->x *= mInvScale;
        pRow->y *= mInvScale;
        pRow->z *= mInvScale;
    }
    const Transform saved = ToTransform(pCam->mLocalXfm);
    Transform armToCam;
    Multiply(armToCam, InvertScaled(ToTransform(pCam->mWorldXfm)), arm);
    pCam->SetLocalXfm(InvertScaled(armToCam));
    pCam->UpdateWorldXfm(nullptr, 0);
    pCam->Draw();
    static_cast<Rnd::Drawable *>(mSys)->Draw();
    pCam->SetLocalXfm(saved);
    CamSlide::sSlide->UpdateWorldXfm(nullptr, 0);
    pCam->Draw();
}
