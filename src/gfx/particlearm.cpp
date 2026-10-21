#include "gfx/particlearm.h"

#include "gfx/camslide.h"
#include "math/transform.h"
#include "rnd/rndcam.h"

namespace {

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

void ParticleArm::Init(RndParticleSys *pSys, RndTransformable *pTrans, float fScale) {
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

void ParticleArm::Scale(RndParticleSys *pSys, float fScale) {
    pSys->GetSize().x *= fScale;
    pSys->GetSize().y *= fScale;
    pSys->GetSpeed().x *= fScale;
    pSys->GetSpeed().y *= fScale;
    pSys->GetPosLow().x *= fScale;
    pSys->GetPosLow().y *= fScale;
    pSys->GetPosLow().z *= fScale;
    pSys->GetPosHigh().x *= fScale;
    pSys->GetPosHigh().y *= fScale;
    pSys->GetPosHigh().z *= fScale;
    pSys->GetBubbleSize().x *= fScale;
    pSys->GetBubbleSize().y *= fScale;
    pSys->GetLocalXfm().mTranslation.x *= fScale;
    pSys->GetLocalXfm().mTranslation.y *= fScale;
    pSys->GetLocalXfm().mTranslation.z *= fScale;
    pSys->SetDirty(1);
}

void ParticleArm::Draw() {
    RndCam *pCam = RndCam::sCurrent;
    if (pCam == nullptr) {
        return;
    }
    Transform arm = mTrans->GetWorldXfm();
    Vector3 *basis[] = {&arm.mBasisX, &arm.mBasisY, &arm.mBasisZ};
    for (Vector3 *pRow : basis) {
        pRow->x *= mInvScale;
        pRow->y *= mInvScale;
        pRow->z *= mInvScale;
    }
    const Transform saved = pCam->GetLocalXfm();
    Transform armToCam;
    Multiply(armToCam, InvertScaled(pCam->GetWorldXfm()), arm);
    pCam->SetLocalXfm(InvertScaled(armToCam));
    pCam->SetDirty(1);
    pCam->UpdateWorldXfm(nullptr, 0);
    pCam->Draw();
    static_cast<RndDrawable *>(mSys)->Draw();
    pCam->SetLocalXfm(saved);
    pCam->SetDirty(1);
    CamSlide::sSlide->UpdateWorldXfm(nullptr, 0);
    pCam->Draw();
}
