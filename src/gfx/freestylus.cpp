#include "gfx/freestylus.h"

#include <cmath>

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxutil.h"
#include "math/sine.h"
#include "rnd/rndmanager.h"

namespace {

// The pose of the beams before they grow and of a new sample.
constexpr float kBeamGrowStart = 0.0f;
constexpr float kSampleStart = 500.0f;

// The speed the beams and the samples pose at.
constexpr float kPoseSpeed = 1.0f;

// The substeps of the particles of the button.
constexpr int kButtonSteps = 8;

// The frame of `freestylus axe to scratch.mnm` for a scratch freestyle.
constexpr float kScratchFrame = 10000.0f;

// The flash plays for this long.
constexpr float kFlashLength = 500.0f;

// The frames the pitch and the colour cover from 0 to 1.
constexpr float kPoseFrames = 1000.0f;

// A quarter turn, which turns a sine into a cosine.
constexpr float kQuarterTurn = 1.5707964f;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

// Apply three basis rows to a direction, keeping the fourth word of the direction.
inline Vector3 RotateDir(const Vector3 *pBasis, const Vector3 &dir) {
    return Vector3{(pBasis[0].x * dir.x) + (pBasis[1].x * dir.y) + (pBasis[2].x * dir.z),
                   (pBasis[0].y * dir.x) + (pBasis[1].y * dir.y) + (pBasis[2].y * dir.z),
                   (pBasis[0].z * dir.x) + (pBasis[1].z * dir.y) + (pBasis[2].z * dir.z),
                   dir.w};
}

// Scale the three components of a vector to unit length, keeping the fourth word.
inline void Normalize(Vector3 *pVec) {
    const float flScale =
        1.0f / std::sqrt((pVec->x * pVec->x) + (pVec->y * pVec->y) + (pVec->z * pVec->z));
    pVec->x *= flScale;
    pVec->y *= flScale;
    pVec->z *= flScale;
}

// Write the three basis rows of a transform as a uniform scale, keeping the translation.
inline void SetScaleBasis(RndTransformable *pTrans, float flScale) {
    Transform &xfm = pTrans->mLocalXfm;
    xfm.mBasisX.x = flScale;
    xfm.mBasisX.y = 0.0f;
    xfm.mBasisX.z = 0.0f;
    xfm.mBasisY.x = 0.0f;
    xfm.mBasisY.y = flScale;
    xfm.mBasisY.z = 0.0f;
    xfm.mBasisZ.x = 0.0f;
    xfm.mBasisZ.y = 0.0f;
    xfm.mBasisZ.z = flScale;
    pTrans->mDirty = 1;
}

} // namespace

float Freestylus::sTumbleAnglePerMs = 0.001f;

Freestylus::Freestylus(GfxTunnel *, int nType)
    : mBeamGrow(kBeamGrowStart,
                FindObject<RndAnimatable>("freestylus beamgrow.mnm"),
                nullptr,
                kPoseSpeed),
      mSample(
          kSampleStart, FindObject<RndAnimatable>("freestylus sample.tnm"), nullptr, kPoseSpeed),
      mButton(kButtonSteps, nullptr) {
    mTime = 0.0f;
    mFlashTime = kNoFlash;
    mScale = kNotSetUp;
    mView = FindObject<RndView>("freestylus.view");
    mTempo = FindObject<RndView>("freestylus tempo.view");
    mTumble = FindObject<RndView>("freestylus tumble.view");
    mBeams = FindObject<RndView>("freestylus beams.view");
    mPitch = FindObject<RndAnimatable>("freestylus pitch.view");
    mColor = FindObject<RndAnimatable>("freestylus color.mnm");
    mFlash = FindObject<RndView>("freestylus flash.view");
    FindObject<RndAnimatable>("freestylus axe to scratch.mnm")
        ->SetFrame((nType == kTypeScratch) ? kScratchFrame : 0.0f);
    mParticles = FindObject<RndParticleSys>("freestylus.part");
    mButton.mSys = FindObject<RndParticleSys>("freestylus button 0.part");
    mButton.mSys->SetFrameSelf(mTime);
    mButton.mSys->FreeAllParticles();
    mBasis[0].x = 1.0f;
    mBasis[0].z = 0.0f;
    mBasis[0].y = 0.0f;
    mBasis[1].x = 0.0f;
    mBasis[1].z = 0.0f;
    mBasis[1].y = 1.0f;
    mBasis[2].x = 0.0f;
    mBasis[2].z = 1.0f;
    mBasis[2].y = 0.0f;
}

Freestylus::~Freestylus() = default;

void Freestylus::Setup(float flScale) {
    if (mScale != kNotSetUp) {
        return;
    }
    mScale = flScale;
    const float flSize = flScale * TheGfxManager.mScrollSpeed;
    mButtonForce = mButton.mSys->mForce;
    ScaleParticles(mButton.mSys, flSize);
    mArm.Init(FindObject<RndParticleSys>("freestylus2.part"), mView, flSize);
    mArm.Scale(mParticles, flSize);
    mView->RemoveDraw(mArm.mSys);
    SetScaleBasis(mFlash, TheGfxManager.mScrollSpeed);
}

void Freestylus::Reset() {
    mFlashTime = kNoFlash;
    mButton.Reset();
}

void Freestylus::SetPitchColor(float flPitch, float flColor) {
    mPitch->SetFrame(flPitch * kPoseFrames);
    mColor->SetFrame(flColor * kPoseFrames);
}

void Freestylus::Spin(float flTime, float flTumbleX, float flTumbleY) {
    const float flAngle = flTime * sTumbleAnglePerMs;

    const float flPitch = flAngle * flTumbleY;
    const float flPitchCos = SinApprox(flPitch + kQuarterTurn);
    const float flPitchSin = SinApprox(flPitch);
    const Vector3 pitch[] = {Vector3{1.0f, 0.0f, 0.0f},
                             Vector3{0.0f, flPitchCos, flPitchSin},
                             Vector3{0.0f, -flPitchSin, flPitchCos}};
    for (auto &row : mBasis) {
        row = RotateDir(pitch, row);
    }

    const float flYaw = flAngle * flTumbleX;
    const float flYawCos = SinApprox(flYaw + kQuarterTurn);
    const float flYawSin = SinApprox(flYaw);
    const Vector3 yaw[] = {Vector3{flYawCos, 0.0f, -flYawSin},
                           Vector3{0.0f, 1.0f, 0.0f},
                           Vector3{flYawSin, 0.0f, flYawCos}};
    for (auto &row : mBasis) {
        row = RotateDir(yaw, row);
    }

    Normalize(&mBasis[1]);
    CrossVec3(&mBasis[1].x, &mBasis[2].x, &mBasis[0].x);
    Normalize(&mBasis[0]);
    CrossVec3(&mBasis[0].x, &mBasis[1].x, &mBasis[2].x);
    mTumble->mLocalXfm.mBasisX = mBasis[0];
    mTumble->mLocalXfm.mBasisY = mBasis[1];
    mTumble->mLocalXfm.mBasisZ = mBasis[2];
    mTumble->mDirty = 1;
}

void Freestylus::Flash() {
    mFlashTime = TheGameDb->mSongTime;
}

void Freestylus::ClearParticles() {
    const float flTime = TheGameDb->mSongTime;
    mParticles->SetFrameSelf(flTime);
    mParticles->FreeAllParticles();
    mArm.mSys->SetFrameSelf(flTime);
    mArm.mSys->FreeAllParticles();
}

void Freestylus::Update(bool bPlaying, float, float flTime, float flScale) {
    if (bPlaying) {
        mTime += flTime;
    }
    mBeamGrow.Update(flTime, false);
    mSample.Update(flTime, false);
    SetScaleBasis(mView, flScale * mScale);
}

void Freestylus::Draw(RndTransformable *pParent, int) {
    bool bFlash = false;
    const float flTime = TheGameDb->mSongTime;
    if (mFlashTime != kNoFlash) {
        const float flSinceFlash = flTime - mFlashTime;
        if (kFlashLength <= flSinceFlash) {
            mFlashTime = kNoFlash;
        } else {
            mFlash->SetFrame(flSinceFlash);
            bFlash = true;
        }
    }
    const Transform &world = pParent->mWorldXfm;
    const Vector3 basis[] = {world.mBasisX, world.mBasisY, world.mBasisZ};
    mButton.mSys->mForce = RotateDir(basis, mButtonForce);
    mView->SetFrame(TheGameDb->mSongTime);
    mTempo->SetFrame(TheGameDb->mSongTick);
    mView->UpdateWorldXfm(pParent, 1);
    mFlash->UpdateWorldXfm(pParent, 1);
    mParticles->SetFrameSelf(flTime);
    mArm.mSys->SetFrameSelf(flTime);
    mButton.Update(flTime, mTime);
    mBeams->SetShowing(mBeamGrow.mValue != 0.0f);
    mView->Draw();
    mArm.Draw();
    if (bFlash) {
        mFlash->Draw();
    }
}
