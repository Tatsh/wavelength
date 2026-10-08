#include "rnd/rndcam.h"

#include <cmath>

#include "os/debug.h"
#include "rnd/rndmanager.h"
#include "rnd/rndrenderer.h"

namespace {

constexpr float kDefaultNearPlane = 1.0f;
constexpr float kDefaultFarPlane = 1000.0f;
constexpr float kDefaultFov = 1.5707964f;
// The near plane is never closer than this fraction of the far plane.
constexpr float kNearPlaneMinRatio = 1000.0f;
// Where CreateDefault() places the default camera along y.
constexpr float kDefaultCamDistance = -150.0f;

// The first version with each later field, and the versions that store a value nothing reads.
constexpr int kRevCollideable = 8;
constexpr int kRevUnusedAfterFov = 2;
constexpr int kRevFirstUnusedAfterRect = 1;
constexpr int kRevLastUnusedAfterRect = 2;
constexpr int kRevZRange = 4;
constexpr int kRevTargetTex = 5;
constexpr int kRevUnusedAfterTargetTex = 6;

// A plane rotated by the basis of a transform, its distance moved by the translation.
Plane TransformPlane(const Transform &xfm, const Plane &plane) {
    Plane out;
    out.a = xfm.mBasisX.x * plane.a + xfm.mBasisY.x * plane.b + xfm.mBasisZ.x * plane.c;
    out.b = xfm.mBasisX.y * plane.a + xfm.mBasisY.y * plane.b + xfm.mBasisZ.y * plane.c;
    out.c = xfm.mBasisX.z * plane.a + xfm.mBasisY.z * plane.b + xfm.mBasisZ.z * plane.c;
    out.d = plane.d -
            (xfm.mTranslation.x * out.a + xfm.mTranslation.y * out.b + xfm.mTranslation.z * out.c);
    return out;
}

void Normalize(const Vector3 &v, Vector3 &out) {
    const float fScale = 1.0f / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    out.x = v.x * fScale;
    out.y = v.y * fScale;
    out.z = v.z * fScale;
}

void Cross(const Vector3 &a, const Vector3 &b, Vector3 &out) {
    const float fX = a.y * b.z - a.z * b.y;
    const float fY = a.z * b.x - a.x * b.z;
    const float fZ = a.x * b.y - a.y * b.x;
    out.x = fX;
    out.y = fY;
    out.z = fZ;
}

void ReadFloat(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

void WriteFloat(BinStream &stream, float fValue) {
    stream.WriteEndian(&fValue, sizeof(fValue));
}

const Vector3 kZeroRow{0.0f, 0.0f, 0.0f, 1.0f};

void ClearXfm(Transform &xfm) {
    xfm.mBasisX = kZeroRow;
    xfm.mBasisY = kZeroRow;
    xfm.mBasisZ = kZeroRow;
    xfm.mTranslation = kZeroRow;
}

} // namespace

RndCam *RndCam::sCurrent = nullptr;
RndCam *RndCam::sDefault = nullptr;
const char *RndCam::sClassName = "Cam";
int RndCam::sRev = 8;

RndCam::RndCam(const char *pszName) : RndObject(pszName) {
    mFarPlane = kDefaultFarPlane;
    mFov = kDefaultFov;
    mNearPlane = kDefaultNearPlane;
    mZRange.x = 0.0f;
    mZRange.y = 1.0f;
    mScreenRect.x = 0.0f;
    mScreenRect.h = 1.0f;
    mScreenRect.w = 1.0f;
    mScreenRect.y = 0.0f;
    mTargetTex = nullptr;
    AcquireTargetTex();
}

RndCam::~RndCam() {
    if (sCurrent == this) {
        sCurrent = nullptr;
    }
    ReleaseTargetTex();
}

void RndCam::ListDrawObjects(std::list<RndObject *> &objects) {
    objects.push_back(this);
}

void RndCam::ListDrawables(std::list<RndDrawable *> &drawables) {
    sCurrent = this;
    RndDrawable::ListDrawables(drawables);
}

int RndCam::DrawShowing() {
    sCurrent = this;
    return 1;
}

int RndCam::UpdateWorldXfm(RndTransformable *pParent, int bForce) {
    if (RndTransformable::UpdateWorldXfm(pParent, bForce) == 0) {
        return 0;
    }
    UpdateWorldProject();
    return 1;
}

void RndCam::Collide(const Vector2 &point, std::list<Collision> &collisions) {
    if (mShowing != 0 && mScreenRect.x < point.x && point.x < mScreenRect.x + mScreenRect.w &&
        mScreenRect.y < point.y && point.y < mScreenRect.y + mScreenRect.h) {
        collisions.push_back(Collision{this, 0.0f});
    }
    RndCollideable::Collide(point, collisions);
}

void RndCam::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndTransformable::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndCollideable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndCam]\n";
    stream << "nearPlane:" << mNearPlane << " farPlane:" << mFarPlane << " fov:" << mFov << "\n";
    stream << "screenRect:" << mScreenRect << "\n";
    stream << "zRange:" << mZRange << " targetTex:" << mTargetTex << "\n";
    if (stream.mDumpLevel < 2) {
        return;
    }
    stream << "localProject:" << mLocalProject << "\n";
    stream << "worldProject:" << mWorldProject << "\n";
    stream << "localFrustrum:" << mLocalFrustum << "\n";
    stream << "worldFrustrum:" << mWorldFrustum << "\n";
    stream << "invWorldProject:" << mInvWorldProject << "\n";
}

void RndCam::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndTransformable::Save(stream);
    RndDrawable::Save(stream);
    RndCollideable::Save(stream);
    WriteFloat(stream, mNearPlane);
    WriteFloat(stream, mFarPlane);
    WriteFloat(stream, mFov);
    WriteFloat(stream, mScreenRect.x);
    WriteFloat(stream, mScreenRect.y);
    WriteFloat(stream, mScreenRect.w);
    WriteFloat(stream, mScreenRect.h);
    WriteFloat(stream, mZRange.x);
    WriteFloat(stream, mZRange.y);
    stream.WriteString(mTargetTex != nullptr ? mTargetTex->mName.c_str() : "");
}

void RndCam::Replace(RndObject *pFrom, RndObject *pTo) {
    RndTransformable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    RndCollideable::Replace(pFrom, pTo);
    if (mTargetTex != pFrom) {
        return;
    }
    if (mTargetTex != nullptr) {
        mTargetTex->RemoveRef(this);
    }
    if (mTargetTex != nullptr) {
        mTargetTex = pTo != nullptr ? dynamic_cast<RndTex *>(pTo) : nullptr;
    }
    if (mTargetTex != nullptr) {
        mTargetTex->AddRef(this);
    }
}

void RndCam::Copy(const RndObject *pSource, int nFlags) {
    const RndCam *pCam = pSource != nullptr ? dynamic_cast<const RndCam *>(pSource) : nullptr;
    RndTransformable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    RndCollideable::Copy(pSource, nFlags);
    ReleaseTargetTex();
    mNearPlane = pCam->mNearPlane;
    mFarPlane = pCam->mFarPlane;
    mFov = pCam->mFov;
    mScreenRect = pCam->mScreenRect;
    mZRange = pCam->mZRange;
    mTargetTex = pCam->mTargetTex;
    AcquireTargetTex();
}

void RndCam::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Cam");
        return;
    }
    RndTransformable::Load(stream);
    RndDrawable::Load(stream);
    if (nRev >= kRevCollideable) {
        RndCollideable::Load(stream);
    }
    ReleaseTargetTex();
    ReadFloat(stream, mNearPlane);
    ReadFloat(stream, mFarPlane);
    ReadFloat(stream, mFov);
    if (nRev < kRevUnusedAfterFov) {
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused));
    }
    ReadFloat(stream, mScreenRect.x);
    ReadFloat(stream, mScreenRect.y);
    ReadFloat(stream, mScreenRect.w);
    ReadFloat(stream, mScreenRect.h);
    if (nRev >= kRevFirstUnusedAfterRect && nRev <= kRevLastUnusedAfterRect) {
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused));
    }
    if (nRev >= kRevZRange) {
        ReadFloat(stream, mZRange.x);
        ReadFloat(stream, mZRange.y);
    }
    if (nRev >= kRevTargetTex) {
        String name;
        stream >> name;
        if (name.mLength == 0) {
            mTargetTex = nullptr;
        } else {
            mTargetTex = dynamic_cast<RndTex *>(TheManager.Find(name.c_str()));
        }
    }
    if (nRev == kRevUnusedAfterTargetTex) {
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused));
    }
    AcquireTargetTex();
}

void RndCam::WorldToScreen(const Vector3 &world, Vector2 &screen) const {
    const Transform &xfm = mWorldProject;
    const float fX = xfm.mBasisX.x * world.x + xfm.mBasisY.x * world.y + xfm.mBasisZ.x * world.z +
                     xfm.mTranslation.x;
    const float fY = xfm.mBasisX.y * world.x + xfm.mBasisY.y * world.y + xfm.mBasisZ.y * world.z +
                     xfm.mTranslation.y;
    const float fZ = xfm.mBasisX.z * world.x + xfm.mBasisY.z * world.y + xfm.mBasisZ.z * world.z +
                     xfm.mTranslation.z;
    if (fZ == 0.0f) {
        screen.x = fX;
        screen.y = fY;
    } else {
        const float fInverse = 1.0f / fZ;
        screen.x = fX * fInverse;
        screen.y = fY * fInverse;
    }
    screen.y = (screen.y + 1.0f) * 0.5f;
    screen.x = (screen.x + 1.0f) * 0.5f;
    screen.x = mScreenRect.x + screen.x * mScreenRect.w;
    screen.y = mScreenRect.y + screen.y * mScreenRect.h;
}

void RndCam::SetTargetTex(RndTex *pTex) {
    if (mTargetTex != nullptr) {
        mTargetTex->RemoveRef(this);
    }
    mTargetTex = pTex;
    if (pTex != nullptr) {
        pTex->AddRef(this);
    }
    UpdateProjection();
}

void RndCam::SetFrustum(float fNear, float fFar, float fFov) {
    mFov = fFov;
    mNearPlane = fNear;
    mFarPlane = fFar;
    const float fMinNear = fFar / kNearPlaneMinRatio;
    mNearPlane = fNear < fMinNear ? fMinNear : fNear;
    UpdateProjection();
}

void RndCam::UpdateProjection() {
    float fAspect = mScreenRect.h / mScreenRect.w;
    if (mTargetTex != nullptr) {
        fAspect *= static_cast<float>(mTargetTex->mHeight) / static_cast<float>(mTargetTex->mWidth);
    } else {
        fAspect *= TheRnd->AspectRatio();
    }
    mLocalFrustum.Set(mNearPlane, mFarPlane, mFov, fAspect);
    ClearXfm(mLocalProject);
    ClearXfm(mInvLocalProject);
    if (mFov == 0.0f) {
        const float fScaleY = -2.0f / fAspect;
        const float fInvScaleY = fAspect * -0.5f;
        mLocalProject.mTranslation.y = -1.0f;
        mLocalProject.mBasisX.x = 2.0f;
        mInvLocalProject.mTranslation.x = 0.5f;
        mLocalProject.mBasisZ.y = fScaleY;
        mInvLocalProject.mTranslation.z = fInvScaleY;
        mLocalProject.mTranslation.x = -1.0f;
        mInvLocalProject.mBasisX.x = 0.5f;
        mInvLocalProject.mBasisY.z = fInvScaleY;
    } else {
        const float fFocal = 1.0f / std::tan(mFov * 0.5f);
        mInvLocalProject.mBasisZ.y = 1.0f;
        mLocalProject.mBasisY.z = 1.0f;
        mLocalProject.mBasisX.x = fFocal;
        mInvLocalProject.mBasisY.z = -fAspect / fFocal;
        mInvLocalProject.mBasisX.x = 1.0f / fFocal;
        mLocalProject.mBasisZ.y = -fFocal / fAspect;
    }
    UpdateWorldProject();
}

void RndCam::UpdateWorldProject() {
    Normalize(mWorldXfm.mBasisY, mOrthoWorldXfm.mBasisY);
    Cross(mOrthoWorldXfm.mBasisY, mWorldXfm.mBasisZ, mOrthoWorldXfm.mBasisX);
    Normalize(mOrthoWorldXfm.mBasisX, mOrthoWorldXfm.mBasisX);
    Cross(mOrthoWorldXfm.mBasisX, mOrthoWorldXfm.mBasisY, mOrthoWorldXfm.mBasisZ);
    mOrthoWorldXfm.mTranslation = mWorldXfm.mTranslation;
    Invert(mInvWorldXfm, mOrthoWorldXfm);
    mWorldFrustum.mFront = TransformPlane(mOrthoWorldXfm, mLocalFrustum.mFront);
    mWorldFrustum.mBack = TransformPlane(mOrthoWorldXfm, mLocalFrustum.mBack);
    mWorldFrustum.mLeft = TransformPlane(mOrthoWorldXfm, mLocalFrustum.mLeft);
    mWorldFrustum.mRight = TransformPlane(mOrthoWorldXfm, mLocalFrustum.mRight);
    mWorldFrustum.mTop = TransformPlane(mOrthoWorldXfm, mLocalFrustum.mTop);
    mWorldFrustum.mBottom = TransformPlane(mOrthoWorldXfm, mLocalFrustum.mBottom);
    Multiply(mWorldProject, mLocalProject, mInvWorldXfm);
    Multiply(mInvWorldProject, mOrthoWorldXfm, mInvLocalProject);
}

void RndCam::CreateDefault() {
    RndObject *pObject = TheManager.Create(sClassName, "[default cam]");
    RndCam *pCam = pObject != nullptr ? dynamic_cast<RndCam *>(pObject) : nullptr;
    sDefault = pCam;
    pCam->mInternal = 1;
    pCam->mDirty = 1;
    // Yes, the binary leaves the fourth word of the translation unset.
    pCam->mLocalXfm.mTranslation.x = 0.0f;
    pCam->mLocalXfm.mTranslation.y = kDefaultCamDistance;
    pCam->mLocalXfm.mTranslation.z = 0.0f;
}

float RndCam::ConvertFov(float fFrom, float fTo, float fFov) {
    return std::atan((std::tan(fFov * 0.5f) * fFrom) / fTo) * 2.0f;
}

void RndCam::ReleaseTargetTex() {
    if (mTargetTex != nullptr) {
        mTargetTex->RemoveRef(this);
    }
}

void RndCam::AcquireTargetTex() {
    if (mTargetTex != nullptr) {
        mTargetTex->AddRef(this);
    }
    UpdateProjection();
}
