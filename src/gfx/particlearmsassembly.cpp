#include "gfx/particlearmsassembly.h"

#include <memory>

#include "math/sine.h"
#include "math/transform.h"
#include "math/transformops.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/debug.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"

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
    : mPath(nullptr), mNumArms(1), mRotationSpeed(0.0f), mArmLength(1.0f), mTick(0.0f),
      mAngle(0.0f) {
    mSys.reset(dynamic_cast<RndParticleSys *>(
        TheManager.Create(RndParticleSys::sClassName, pData->Sym(0))));
    mSys->SetMode(RndParticleSys::kModeSprite);
    const char *pszParent = kDefaultDrawParent;
    pData->FindSymbol("draw_parent", &pszParent, false);
    dynamic_cast<RndDrawable *>(TheManager.Find(pszParent))->AddDraw(mSys.get(), nullptr);
    LoadConfig(pData);
}

ParticleArmsAssembly::~ParticleArmsAssembly() = default;

void ParticleArmsAssembly::LoadConfig(DataArray *pData) {
    mSys->FreeAllParticles();
    mSys->GetEmitRate().y = 0.0f;
    mSys->GetEmitRate().x = 0.0f;
    DataArray *pTicksAhead = pData->FindArray("ticks_ahead", false);
    if (pTicksAhead != nullptr) {
        mTicksAhead.reset(ObjectToInterpolator(pTicksAhead->Array(1)));
    } else {
        mTicksAhead = std::make_unique<LinearInterpolator>(
            kDefaultTicksAhead, kDefaultTicksAhead, 0.0f, 1.0f);
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
        mPath = dynamic_cast<RndTransAnim *>(TheManager.Find(pszName));
    }

    DataArray *pSys = pData->FindArray("particle_sys", true);
    if (pSys == nullptr) {
        return;
    }
    if (pSys->FindSymbol("mat", &pszName, false)) {
        mSys->SetMat(dynamic_cast<RndMat *>(TheManager.Find(pszName)));
    }
    int nValue;
    if (pSys->FindSymbol("particles_owner", &pszName, false)) {
        mSys->SetParticlesOwner(dynamic_cast<RndParticleSys *>(TheManager.Find(pszName)));
    } else if (pSys->FindInt("max_particles", &nValue, false)) {
        mSys->SetNumParticles(nValue);
    }

    Vector2 range;
    if (pSys->FindVector("emit_rate", &range, false)) {
        mSys->GetEmitRate().x = range.x;
        mSys->GetEmitRate().y = range.y;
    }
    if (pSys->FindVector("life", &range, false)) {
        mSys->GetLife() = range;
    }
    if (pSys->FindVector("speed", &range, false)) {
        mSys->GetSpeed() = range;
    }
    if (pSys->FindVector("pitch", &range, false)) {
        mSys->GetPitch() = ToRadians(range);
    }
    if (pSys->FindVector("yaw", &range, false)) {
        mSys->GetYaw() = ToRadians(range);
    }
    if (pSys->FindVector("size", &range, false)) {
        mSys->GetSize().x = range.x;
        mSys->GetSize().y = range.y;
    }

    DataArray *pRange = pSys->FindArray("pos", false);
    if (pRange != nullptr) {
        const Vector3 low = ReadVector3(pRange->Array(1));
        const Vector3 high = ReadVector3(pRange->Array(2));
        mSys->GetPosLow() = low;
        mSys->GetPosHigh() = high;
    }
    pRange = pSys->FindArray("start_color", false);
    if (pRange != nullptr) {
        const Color low = ReadColor(pRange->Array(1));
        const Color high = ReadColor(pRange->Array(2));
        mSys->GetStartColorLow() = low;
        mSys->GetStartColorHigh() = high;
    }
    pRange = pSys->FindArray("end_color", false);
    if (pRange != nullptr) {
        const Color low = ReadColor(pRange->Array(1));
        const Color high = ReadColor(pRange->Array(2));
        mSys->GetEndColorLow() = low;
        mSys->GetEndColorHigh() = high;
    }
    DataArray *pBubble = pSys->FindArray("bubble", false);
    if (pBubble != nullptr) {
        mSys->SetBubble(1);
        if (pBubble->FindVector("period", &range, false)) {
            mSys->GetBubblePeriod() = range;
        }
        if (pBubble->FindVector("size", &range, false)) {
            mSys->GetBubbleSize() = range;
        }
    }
    Vector3 force;
    if (pSys->FindVector("force", &force, false)) {
        mSys->GetForce() = force;
    }
    if (pSys->FindInt("read_z", &nValue, false)) {
        mSys->SetReadZ((nValue != 0));
    }
}

void ParticleArmsAssembly::Poll(TnlGeom *pGeom, float flTick) {
    RndParticleSys *pSys = mSys.get();
    if (pSys->GetLiveParticles() == nullptr && pSys->GetEmitRate().x == 0.0f &&
        pSys->GetEmitRate().y == 0.0f) {
        mTick = flTick;
        pSys->ResetEmitFrame();
        return;
    }

    float flDelta = flTick - mTick;
    mAngle += flDelta * mRotationSpeed;
    Transform base;
    if (mPath != nullptr) {
        mPath->MakeTransform(mTicksAhead->Eval(flTick), base, 1);
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

        pSys->GetLocalXfm() = arm;

        pSys->SetDirty(1);
        pSys->UpdateWorldXfm(nullptr, 0);

        mTick += flDelta;
        pSys->SetEmitFrame(mTick);
    }
    pSys->SetFrame(mTick);
}

void ParticleArmsAssembly::Move(float flFromTicks, float flToTicks, float flTicks) {
    mTicksAhead->Reset(flFromTicks, flToTicks, mTick, mTick + flTicks);
}

void ParticleArmsAssembly::Report() {
    const char *pszName = mSys->GetName();
    const int nLive = mSys->NumLiveParticles();
    RndParticleSys *pOwner = mSys->GetParticlesOwner();
    const int nPool = pOwner->GetNumParticles();
    const char *pszOwner = (pOwner == mSys.get()) ? "self" : pOwner->GetName();
    DebugPrint(
        "particle arm %s: %d particles of %d (owner: %s)\n", pszName, nLive, nPool, pszOwner);
}

const char *ParticleArmsAssembly::Name() const {
    return mSys->GetName();
}
