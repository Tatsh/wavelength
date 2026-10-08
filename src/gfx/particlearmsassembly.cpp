#include "gfx/particlearmsassembly.h"

#include "math/sine.h"
#include "math/transform.h"
#include "math/transformops.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/debug.h"
#include "rnd/drawable.h"
#include "rnd/manager.h"
#include "rnd/mat.h"

namespace {

constexpr float kPi = 3.14159274f;
constexpr float kHalfPi = 1.57079637f;
constexpr float kDegreesPerHalfTurn = 180.0f;

// The curve of the ticks ahead when the entry gives none, a constant 500 ticks.
constexpr float kDefaultTicksAhead = 500.0f;

// The arm transform puts -0.0 where the arithmetic of the original produced it.
constexpr float kNegativeZero = -0.0f;

const char *const kDefaultDrawParent = "particle arms draw.view";

// Read the first three numbers of a row.
inline Vector3 ReadVector3(DataArray *pRow) {
    const float flX = pRow->Float(0);
    const float flY = pRow->Float(1);
    return Vector3{flX, flY, pRow->Float(2)};
}

inline Color ReadColor(DataArray *pRow) {
    const float flR = pRow->Float(0);
    const float flG = pRow->Float(1);
    const float flB = pRow->Float(2);
    return Color{flR, flG, flB, pRow->Float(3)};
}

inline Vector2 ToRadians(const Vector2 &degrees) {
    return Vector2{(degrees.x * kPi) / kDegreesPerHalfTurn,
                   (degrees.y * kPi) / kDegreesPerHalfTurn};
}

} // namespace

ParticleArmsAssembly::ParticleArmsAssembly(DataArray *pData)
    : mPath(nullptr), mTicksAhead(nullptr), mNumArms(1), mRotationSpeed(0.0f), mArmLength(1.0f),
      mTick(0.0f), mAngle(0.0f) {
    mSys = dynamic_cast<Rnd::ParticleSys *>(
        Rnd::TheManager.Create(Rnd::g_particleSysClassName.mStr, pData->Sym(0)));
    mSys->mMode = Rnd::ParticleSys::kModeSprite;
    const char *pszParent = kDefaultDrawParent;
    pData->FindSymbol("draw_parent", &pszParent, false);
    dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find(pszParent))->AddDraw(mSys, nullptr);
    LoadConfig(pData);
}

ParticleArmsAssembly::~ParticleArmsAssembly() {
    delete mSys;
    delete mTicksAhead;
}

void ParticleArmsAssembly::LoadConfig(DataArray *pData) {
    mSys->FreeAllParticles();
    mSys->mEmitRateHigh = 0.0f;
    mSys->mEmitRateLow = 0.0f;
    delete mTicksAhead;
    DataArray *pTicksAhead = pData->FindArray("ticks_ahead", false);
    if (pTicksAhead != nullptr) {
        mTicksAhead = ObjectToInterpolator(pTicksAhead->Array(1));
    } else {
        mTicksAhead = new LinearInterpolator(kDefaultTicksAhead, kDefaultTicksAhead, 0.0f, 1.0f);
    }
    pData->FindInt("num_arms", &mNumArms, false);
    pData->FindFloat("first_arm_angle", &mAngle, false);
    pData->FindFloat("arm_length", &mArmLength, false);
    pData->FindFloat("rotation_speed", &mRotationSpeed, false);
    mRotationSpeed = (mRotationSpeed * kPi) / kDegreesPerHalfTurn;

    const char *pszName;
    if (!pData->FindSymbol("path", &pszName, false)) {
        mPath = nullptr;
    } else {
        mPath = dynamic_cast<Rnd::TransAnim *>(Rnd::TheManager.Find(pszName));
    }

    DataArray *pSys = pData->FindArray("particle_sys", true);
    if (pSys == nullptr) {
        return;
    }
    if (pSys->FindSymbol("mat", &pszName, false)) {
        mSys->SetMat(dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszName)));
    }
    int nValue;
    if (pSys->FindSymbol("particles_owner", &pszName, false)) {
        mSys->SetParticlesOwner(dynamic_cast<Rnd::ParticleSys *>(Rnd::TheManager.Find(pszName)));
    } else if (pSys->FindInt("max_particles", &nValue, false)) {
        mSys->SetPoolSize(nValue);
    }

    Vector2 range;
    if (pSys->FindVector("emit_rate", &range, false)) {
        mSys->mEmitRateLow = range.x;
        mSys->mEmitRateHigh = range.y;
    }
    if (pSys->FindVector("life", &range, false)) {
        mSys->mLife = range;
    }
    if (pSys->FindVector("speed", &range, false)) {
        mSys->mSpeed = range;
    }
    if (pSys->FindVector("pitch", &range, false)) {
        mSys->mPitch = ToRadians(range);
    }
    if (pSys->FindVector("yaw", &range, false)) {
        mSys->mYaw = ToRadians(range);
    }
    if (pSys->FindVector("size", &range, false)) {
        mSys->mSizeLow = range.x;
        mSys->mSizeHigh = range.y;
    }

    DataArray *pRange = pSys->FindArray("pos", false);
    if (pRange != nullptr) {
        const Vector3 low = ReadVector3(pRange->Array(1));
        const Vector3 high = ReadVector3(pRange->Array(2));
        mSys->mPosLow = low;
        mSys->mPosHigh = high;
    }
    pRange = pSys->FindArray("start_color", false);
    if (pRange != nullptr) {
        const Color low = ReadColor(pRange->Array(1));
        const Color high = ReadColor(pRange->Array(2));
        mSys->mStartColorLow = low;
        mSys->mStartColorHigh = high;
    }
    pRange = pSys->FindArray("end_color", false);
    if (pRange != nullptr) {
        const Color low = ReadColor(pRange->Array(1));
        const Color high = ReadColor(pRange->Array(2));
        mSys->mEndColorLow = low;
        mSys->mEndColorHigh = high;
    }
    DataArray *pBubble = pSys->FindArray("bubble", false);
    if (pBubble != nullptr) {
        mSys->mBubble = 1;
        if (pBubble->FindVector("period", &range, false)) {
            mSys->mBubblePeriod = range;
        }
        if (pBubble->FindVector("size", &range, false)) {
            mSys->mBubbleSize = range;
        }
    }
    Vector3 force;
    if (pSys->FindVector("force", &force, false)) {
        mSys->mForce = force;
    }
    if (pSys->FindInt("read_z", &nValue, false)) {
        mSys->mReadZ = (nValue != 0);
    }
}

void ParticleArmsAssembly::Poll(TnlGeom *pGeom, float flTick) {
    Rnd::ParticleSys *pSys = mSys;
    if (pSys->GetLiveParticles() == nullptr && pSys->mEmitRateLow == 0.0f &&
        pSys->mEmitRateHigh == 0.0f) {
        mTick = flTick;
        pSys->ResetLastFrame();
        return;
    }

    float flDelta = flTick - mTick;
    mAngle += flDelta * mRotationSpeed;
    Transform base;
    if (mPath != nullptr) {
        mPath->EvalFrame(mTicksAhead->Eval(flTick), &base.mBasisX.x, 1);
    } else {
        pGeom->PathXfm(&base, flTick + mTicksAhead->Eval(flTick));
    }
    flDelta /= static_cast<float>(mNumArms);
    for (int i = 0; i < mNumArms; ++i) {
        const float flArmAngle =
            mAngle + ((static_cast<float>(i * 2) * kPi) / static_cast<float>(mNumArms));
        const float flCos = SinApprox(flArmAngle + kHalfPi);
        const float flSin = SinApprox(flArmAngle);
        Transform arm;
        arm.mBasisX = Vector3{-flCos, kNegativeZero, flSin};
        arm.mBasisY = Vector3{kNegativeZero, -1.0f, kNegativeZero};
        arm.mBasisZ = Vector3{flSin, 0.0f, flCos};
        // Yes, the original multiplies the arm length by 0 for the middle component.
        arm.mTranslation = Vector3{flSin * mArmLength, mArmLength * 0.0f, flCos * mArmLength};
        sceVu0MulAffineMatrixXyz(&arm.mBasisX.x, &base.mBasisX.x, &arm.mBasisX.x);

        pSys->SetLocalXfm(arm);
        pSys->UpdateWorldXfm(nullptr, 0);

        mTick += flDelta;
        pSys->Rnd::ParticleSys::SetFrameSelf(mTick);
    }
    pSys->SetFrame(mTick);
}

void ParticleArmsAssembly::Move(float flFromTicks, float flToTicks, float flTicks) {
    mTicksAhead->Reset(flFromTicks, flToTicks, mTick, mTick + flTicks);
}

void ParticleArmsAssembly::Report() {
    const char *pszName = mSys->mName.mStr;
    const int nLive = mSys->NumLiveParticles();
    Rnd::ParticleSys *pOwner = mSys->mParticlesOwner;
    const int nPool = static_cast<int>(pOwner->mParticles.size());
    const char *pszOwner = (pOwner == mSys) ? "self" : pOwner->mName.mStr;
    DebugPrint(
        "particle arm %s: %d particles of %d (owner: %s)\n", pszName, nLive, nPool, pszOwner);
}

const char *ParticleArmsAssembly::Name() const {
    return mSys->mName.mStr;
}
