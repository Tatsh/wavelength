#include "rnd/rndparticlesys.h"

#include "math/frustum.h"
#include "math/rand.h"
#include "math/sine.h"
#include "math/transform.h"
#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtex.h"

namespace {

// The value mLastFrame and mLastEmitFrame hold before their first frame.
constexpr float kUnsetFrame = -9999999.0f;

constexpr float kHalfPi = 1.5707964f;
constexpr float kTwoPi = 6.2831855f;

// The defaults the constructor sets.
constexpr int kDefaultNumParticles = 10;
constexpr float kDefaultBubblePeriod = 10.0f;
constexpr float kDefaultLife = 100.0f;
constexpr float kDefaultEmitRate = 10.0f;
constexpr float kDefaultLineLength = 5.0f;

// The first version with the collision plane in place of a plane list, the line length, the
// bubble motion, the depth test flag, the pool owner, and the line length as a float. Version 8
// alone writes an extra byte.
constexpr int kRevCollidePlane = 2;
constexpr int kRevLineLength = 3;
constexpr int kRevBubble = 4;
constexpr int kRevReadZ = 5;
constexpr int kRevOwner = 6;
constexpr int kRevFloatLineLength = 7;
constexpr int kRevExtraByte = 8;

constexpr int kDumpParticles = 2;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

int ReadFlag(BinStream &stream) {
    unsigned char nFlag;
    stream.Read(&nFlag, sizeof(nFlag));
    return nFlag != 0;
}

void WriteFlag(BinStream &stream, int nValue) {
    const unsigned char nFlag = static_cast<unsigned char>(nValue);
    stream.Write(&nFlag, sizeof(nFlag));
}

void WriteFloat(BinStream &stream, float fValue) {
    stream.WriteEndian(&fValue, sizeof(fValue));
}

void ReadFloat(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

void WriteObjectName(BinStream &stream, const RndObject *pObject) {
    stream.WriteString(pObject != nullptr ? pObject->mName.c_str() : "");
}

} // namespace

const char *RndParticleSys::sClassName = "ParticleSys";
int RndParticleSys::sRev = 9;
int RndParticleSys::sLoadRev;

RndParticleSys::RndParticleSys(const char *pszName) : RndObject(pszName) {
    mNumParticles = kDefaultNumParticles;
    mLastEmitFrame = kUnsetFrame;
    mParticlesOwner = this;
    mParticles = nullptr;
    mLastFrame = kUnsetFrame;
    mBubblePeriod.x = kDefaultBubblePeriod;
    mBubblePeriod.y = kDefaultBubblePeriod;
    mBubbleSize.x = 1.0f;
    mBubbleSize.y = 1.0f;
    mLife.x = kDefaultLife;
    mLife.y = kDefaultLife;
    mPosLow.x = 0.0f;
    mPosLow.z = 0.0f;
    mPosLow.y = 0.0f;
    mPosHigh.x = 0.0f;
    mPosHigh.z = 0.0f;
    mPosHigh.y = 0.0f;
    mSpeed.x = 1.0f;
    mSpeed.y = 1.0f;
    mPitch.x = 0.0f;
    mPitch.y = 0.0f;
    mYaw.x = 0.0f;
    mYaw.y = 0.0f;
    mEmitRate.x = kDefaultEmitRate;
    mEmitRate.y = kDefaultEmitRate;
    mSize.x = 1.0f;
    mSize.y = 1.0f;
    mStartColorLow = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mStartColorHigh = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mEndColorLow = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mEndColorHigh = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mCollide = 0;
    mCollidePlane.a = 0.0f;
    mCollidePlane.c = 1.0f;
    mCollidePlane.d = 0.0f;
    mCollidePlane.b = 0.0f;
    mForce.x = 0.0f;
    mForce.z = 0.0f;
    mForce.y = 0.0f;
    mReadZ = 1;
    mMat = nullptr;
    mBubble = 0;
    mLineLength = kDefaultLineLength;
    mMode = kModeLine;
    // Yes, the binary allocates a pool here that AddObjectRefs() releases at once.
    mParticles = std::make_unique<Particle[]>(mNumParticles);
    AddObjectRefs();
}

RndParticleSys::~RndParticleSys() {
    RemoveObjectRefs();
}

void RndParticleSys::SetNumParticles(int nCount) {
    // Yes, the binary rebuilds the pool of the owner but sets the count of this system.
    mParticlesOwner->RemoveObjectRefs();
    mNumParticles = nCount;
    mParticlesOwner->AddObjectRefs();
}

void RndParticleSys::SetParticlesOwner(RndParticleSys *pOwner) {
    RemoveObjectRefs();
    mParticlesOwner = pOwner;
    AddObjectRefs();
}

void RndParticleSys::Copy(const RndObject *pSource, int nFlags) {
    const RndParticleSys *pSys =
        pSource != nullptr ? dynamic_cast<const RndParticleSys *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    RndTransformable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    RemoveObjectRefs();
    mLife = pSys->mLife;
    mPosLow = pSys->mPosLow;
    mPosHigh = pSys->mPosHigh;
    mSpeed = pSys->mSpeed;
    mPitch = pSys->mPitch;
    mYaw = pSys->mYaw;
    mEmitRate = pSys->mEmitRate;
    mSize = pSys->mSize;
    mStartColorLow = pSys->mStartColorLow;
    mStartColorHigh = pSys->mStartColorHigh;
    mEndColorLow = pSys->mEndColorLow;
    mEndColorHigh = pSys->mEndColorHigh;
    mCollide = pSys->mCollide;
    mCollidePlane = pSys->mCollidePlane;
    mForce = pSys->mForce;
    mMat = pSys->mMat;
    mMode = pSys->mMode;
    mBubblePeriod = pSys->mBubblePeriod;
    mBubbleSize = pSys->mBubbleSize;
    mBubble = pSys->mBubble;
    mReadZ = pSys->mReadZ;
    if ((nFlags & kCopyShareParticles) == 0 && pSys->mParticlesOwner == pSys) {
        mParticlesOwner = this;
        mNumParticles = pSys->mNumParticles;
    } else {
        mParticlesOwner = pSys->mParticlesOwner;
    }
    AddObjectRefs();
}

void RndParticleSys::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    RndTransformable::DumpText(stream);
    RndDrawable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndParticleSys]\n";
    stream << "life:" << mLife << " posLow:" << mPosLow << " posHigh:" << mPosHigh << "\n";
    stream << "speed:" << mSpeed << " pitch:" << mPitch << " yaw:" << mYaw << "\n";
    stream << "emitRate:" << mEmitRate << " size:" << mSize << "\n";
    stream << "startColorLow:" << mStartColorLow << "\n";
    stream << "startColorHigh:" << mStartColorHigh << "\n";
    stream << "endColorLow:" << mEndColorLow << "\n";
    stream << "endColorHigh:" << mEndColorHigh << "\n";
    stream << "collide:" << (mCollide != 0) << " collidePlane:" << mCollidePlane << "\n";
    stream << "force:" << mForce << " mat:" << static_cast<const RndObject *>(mMat)
           << " mode:" << mMode << "\n";
    stream << "numParticles:" << mNumParticles << " lineLength:" << mLineLength << "\n";
    stream << "bubblePeriod:" << mBubblePeriod << " bubbleSize:" << mBubbleSize << "\n";
    const RndObject *pOwner = mParticlesOwner;
    stream << "bubble:" << (mBubble != 0) << " readZ:" << (mReadZ != 0)
           << " particlesOwner:" << pOwner << "\n";
    if (stream.mDumpLevel < kDumpParticles) {
        return;
    }
    stream << "particles:(size: " << mNumParticles << ")";
    for (int i = 0; i < mNumParticles; ++i) {
        stream << "\n" << i << "\t" << mParticles[i];
    }
    stream << "\n";
}

void RndParticleSys::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    RndTransformable::Save(stream);
    RndDrawable::Save(stream);
    WriteFloat(stream, mLife.x);
    WriteFloat(stream, mLife.y);
    WriteFloat(stream, mPosLow.x);
    WriteFloat(stream, mPosLow.y);
    WriteFloat(stream, mPosLow.z);
    WriteFloat(stream, mPosHigh.x);
    WriteFloat(stream, mPosHigh.y);
    WriteFloat(stream, mPosHigh.z);
    WriteFloat(stream, mSpeed.x);
    WriteFloat(stream, mSpeed.y);
    WriteFloat(stream, mPitch.x);
    WriteFloat(stream, mPitch.y);
    WriteFloat(stream, mYaw.x);
    WriteFloat(stream, mYaw.y);
    WriteFloat(stream, mEmitRate.x);
    WriteFloat(stream, mEmitRate.y);
    WriteFloat(stream, mSize.x);
    WriteFloat(stream, mSize.y);
    const Color *colors[] = {&mStartColorLow, &mStartColorHigh, &mEndColorLow, &mEndColorHigh};
    for (const Color *pColor : colors) {
        WriteFloat(stream, pColor->r);
        WriteFloat(stream, pColor->g);
        WriteFloat(stream, pColor->b);
        WriteFloat(stream, pColor->a);
    }
    WriteFlag(stream, mCollide);
    const float fDistance = -mCollidePlane.d;
    WriteFloat(stream, fDistance * mCollidePlane.a);
    WriteFloat(stream, fDistance * mCollidePlane.b);
    WriteFloat(stream, fDistance * mCollidePlane.c);
    WriteFloat(stream, mCollidePlane.a);
    WriteFloat(stream, mCollidePlane.b);
    WriteFloat(stream, mCollidePlane.c);
    WriteFloat(stream, mForce.x);
    WriteFloat(stream, mForce.y);
    WriteFloat(stream, mForce.z);
    WriteObjectName(stream, mMat);
    const int nMode = mMode;
    stream.WriteEndian(&nMode, sizeof(nMode));
    stream.WriteEndian(&mNumParticles, sizeof(mNumParticles));
    WriteFloat(stream, mLineLength);
    WriteFloat(stream, mBubblePeriod.x);
    WriteFloat(stream, mBubblePeriod.y);
    WriteFloat(stream, mBubbleSize.x);
    WriteFloat(stream, mBubbleSize.y);
    WriteFlag(stream, mBubble);
    WriteFlag(stream, mReadZ);
    WriteObjectName(stream, mParticlesOwner);
}

void RndParticleSys::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new ParticleSys");
        return;
    }
    if (sLoadRev > 0) {
        RndAnimatable::Load(stream);
        RndTransformable::Load(stream);
        RndDrawable::Load(stream);
    }
    RemoveObjectRefs();
    ReadFloat(stream, mLife.x);
    ReadFloat(stream, mLife.y);
    ReadFloat(stream, mPosLow.x);
    ReadFloat(stream, mPosLow.y);
    ReadFloat(stream, mPosLow.z);
    ReadFloat(stream, mPosHigh.x);
    ReadFloat(stream, mPosHigh.y);
    ReadFloat(stream, mPosHigh.z);
    ReadFloat(stream, mSpeed.x);
    ReadFloat(stream, mSpeed.y);
    ReadFloat(stream, mPitch.x);
    ReadFloat(stream, mPitch.y);
    ReadFloat(stream, mYaw.x);
    ReadFloat(stream, mYaw.y);
    ReadFloat(stream, mEmitRate.x);
    ReadFloat(stream, mEmitRate.y);
    ReadFloat(stream, mSize.x);
    ReadFloat(stream, mSize.y);
    Color *colors[] = {&mStartColorLow, &mStartColorHigh, &mEndColorLow, &mEndColorHigh};
    for (Color *pColor : colors) {
        ReadFloat(stream, pColor->r);
        ReadFloat(stream, pColor->g);
        ReadFloat(stream, pColor->b);
        ReadFloat(stream, pColor->a);
    }
    if (sLoadRev < kRevCollidePlane) {
        std::list<Plane> planes;
        stream >> planes; // Yes, the binary discards the planes of an early version.
    } else {
        mCollide = ReadFlag(stream);
        Vector3 point;
        ReadFloat(stream, point.x);
        ReadFloat(stream, point.y);
        ReadFloat(stream, point.z);
        ReadFloat(stream, mCollidePlane.a);
        ReadFloat(stream, mCollidePlane.b);
        ReadFloat(stream, mCollidePlane.c);
        mCollidePlane.d = -((point.x * mCollidePlane.a) + (point.y * mCollidePlane.b) +
                            (point.z * mCollidePlane.c));
    }
    ReadFloat(stream, mForce.x);
    ReadFloat(stream, mForce.y);
    ReadFloat(stream, mForce.z);
    mMat = FindByName<RndMat>(stream);
    int nMode;
    stream.ReadEndian(&nMode, sizeof(nMode));
    mMode = static_cast<Mode>(nMode);
    stream.ReadEndian(&mNumParticles, sizeof(mNumParticles));
    if (sLoadRev >= kRevLineLength) {
        if (sLoadRev < kRevFloatLineLength) {
            int nLineLength;
            stream.ReadEndian(&nLineLength, sizeof(nLineLength));
            mLineLength = static_cast<float>(nLineLength);
        } else {
            ReadFloat(stream, mLineLength);
        }
    }
    if (sLoadRev >= kRevBubble) {
        ReadFloat(stream, mBubblePeriod.x);
        ReadFloat(stream, mBubblePeriod.y);
        ReadFloat(stream, mBubbleSize.x);
        ReadFloat(stream, mBubbleSize.y);
        mBubble = ReadFlag(stream);
    }
    if (sLoadRev >= kRevReadZ) {
        mReadZ = ReadFlag(stream);
    }
    if (sLoadRev >= kRevOwner) {
        mParticlesOwner = FindByName<RndParticleSys>(stream);
    }
    if (sLoadRev == kRevExtraByte) {
        unsigned char nUnused;
        stream.Read(&nUnused, sizeof(nUnused)); // Yes, the binary discards this byte.
    }
    AddObjectRefs();
}

void RndParticleSys::AddObjectRefs() {
    if (mMat != nullptr) {
        mMat->AddRef(this);
    }
    if (mParticlesOwner != nullptr) {
        mParticlesOwner->AddRef(this);
    }
    if (mParticlesOwner == this) {
        mParticles = std::make_unique<Particle[]>(mNumParticles);
        Particle *pEnd = mParticles.get() + mNumParticles;
        for (Particle *pParticle = mParticles.get(); pParticle != pEnd; ++pParticle) {
            pParticle->mNext = pParticle + 1;
        }
        mFreeParticles = mParticles.get();
        for (RndParticleSys *pSharer : mSharers) {
            pSharer->mLiveParticles = nullptr;
        }
    } else {
        mParticlesOwner->mSharers.push_back(this);
    }
    mEmitAccumulator = 0.0f;
    mLiveParticles = nullptr;
}

void RndParticleSys::RemoveObjectRefs() {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    if (mParticlesOwner != nullptr) {
        mParticlesOwner->RemoveRef(this);
    }
    FreeAllParticles();
    mParticlesOwner->mSharers.remove(this);
}

void RndParticleSys::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    RndTransformable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    if (static_cast<RndObject *>(mMat) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mMat != nullptr) {
            mMat = pTo != nullptr ? dynamic_cast<RndMat *>(pTo) : nullptr;
        }
        if (mMat != nullptr) {
            mMat->AddRef(this);
        }
    }
    const RndObject *pOwner = mParticlesOwner;
    if (pOwner != pFrom) {
        return;
    }
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    if (mParticlesOwner != nullptr) {
        mParticlesOwner->RemoveRef(this);
    }
    RndObject *pNewOwner = pTo != nullptr ? pTo : static_cast<RndObject *>(this);
    mParticlesOwner = dynamic_cast<RndParticleSys *>(pNewOwner);
    AddObjectRefs();
}

Particle *RndParticleSys::AllocParticle() {
    RndParticleSys *pOwner = mParticlesOwner;
    Particle *pParticle = pOwner->mFreeParticles;
    if (pParticle == pOwner->GetParticles() + pOwner->mNumParticles) {
        return nullptr;
    }
    pOwner->mFreeParticles = pParticle->mNext;
    if (mLiveParticles != nullptr) {
        mLiveParticles->mPrev = pParticle;
    }
    pParticle->mNext = mLiveParticles;
    mLiveParticles = pParticle;
    pParticle->mPrev = pParticle;
    return pParticle;
}

Particle *RndParticleSys::FreeParticle(Particle *pParticle) {
    if (pParticle == nullptr) {
        return nullptr;
    }
    if (pParticle->mPrev == nullptr) {
        TheDebug << "Tried to refree particle from " << mName.c_str() << "\n";
        return nullptr;
    }
    if (pParticle == mLiveParticles) {
        mLiveParticles = pParticle->mNext;
    } else {
        pParticle->mPrev->mNext = pParticle->mNext;
    }
    if (pParticle->mNext != nullptr) {
        pParticle->mNext->mPrev = pParticle->mPrev;
    }
    Particle *pNext = pParticle->mNext;
    pParticle->mNext = mParticlesOwner->mFreeParticles;
    mParticlesOwner->mFreeParticles = pParticle;
    pParticle->mPrev = nullptr;
    return pNext;
}

int RndParticleSys::SetFrameSelf(float fFrame) {
    if (mLastFrame != kUnsetFrame) {
        const float fDelta = fFrame - mLastFrame;
        UpdateParticles(fDelta);
        if (mLastEmitFrame == kUnsetFrame) {
            SpawnParticles(fDelta);
        }
    }
    mLastFrame = fFrame;
    return 1;
}

void RndParticleSys::SetEmitFrame(float fFrame) {
    if (mLastEmitFrame != kUnsetFrame) {
        SpawnParticles(fFrame - mLastEmitFrame);
    }
    mLastEmitFrame = fFrame;
}

void RndParticleSys::ResetEmitFrame() {
    mLastEmitFrame = kUnsetFrame;
}

void RndParticleSys::SpawnParticles(float fDelta) {
    if (fDelta <= 0.0f) {
        return;
    }
    mEmitAccumulator += RandomFloat(mEmitRate.x, mEmitRate.y) * fDelta;
    while (mEmitAccumulator >= 1.0f) {
        Particle *pParticle = AllocParticle();
        if (pParticle == nullptr) {
            mEmitAccumulator = 0.0f;
            return;
        }
        pParticle->mBirthFrame = mFrame;
        pParticle->mDeathFrame = pParticle->mBirthFrame + RandomFloat(mLife.x, mLife.y);

        const float fSpeed = RandomFloat(mSpeed.x, mSpeed.y);
        const float fPitch = RandomFloat(mPitch.x, mPitch.y);
        const float fYaw = RandomFloat(mYaw.x, mYaw.y);
        const float fCosPitch = FastSin(fPitch + kHalfPi);
        pParticle->mVel.x = -fCosPitch * FastSin(fYaw) * fSpeed;
        pParticle->mVel.y = fCosPitch * FastSin(fYaw + kHalfPi) * fSpeed;
        pParticle->mVel.z = FastSin(fPitch) * fSpeed;
        MultiplyVector(mWorldXfm, pParticle->mVel, pParticle->mVel);

        pParticle->mPos.x = RandomFloat(mPosLow.x, mPosHigh.x);
        pParticle->mPos.y = RandomFloat(mPosLow.y, mPosHigh.y);
        pParticle->mPos.z = RandomFloat(mPosLow.z, mPosHigh.z);
        MultiplyPoint(mWorldXfm, pParticle->mPos, pParticle->mPos);

        if (mBubble != 0) {
            pParticle->mBubbleFrequency = kTwoPi / RandomFloat(mBubblePeriod.x, mBubblePeriod.y);
            pParticle->mBubblePhase = RandomFloat(0.0f, kTwoPi);
            const float fAngle = RandomFloat(0.0f, kTwoPi);
            Vector3 direction;
            direction.x = FastSin(fAngle);
            direction.z = FastSin(fAngle + kHalfPi);
            direction.y = 0.0f;
            const float fSize = RandomFloat(mBubbleSize.x, mBubbleSize.y);
            pParticle->mBubbleSize.x = direction.x * fSize;
            pParticle->mBubbleSize.y = direction.y * fSize;
            pParticle->mBubbleSize.z = direction.z * fSize;
            MultiplyVector(mWorldXfm, pParticle->mBubbleSize, pParticle->mBubbleSize);
            const float fSin = FastSin(pParticle->mBubblePhase);
            pParticle->mPos.x += pParticle->mBubbleSize.x * fSin;
            pParticle->mPos.y += pParticle->mBubbleSize.y * fSin;
            pParticle->mPos.z += pParticle->mBubbleSize.z * fSin;
            pParticle->mBubblePhase -= mFrame * pParticle->mBubbleFrequency;
        }

        RandomizeColorAndSize(pParticle);
        Color &colVel = pParticle->mColVel;
        colVel.r = RandomFloat(mEndColorLow.r, mEndColorHigh.r);
        colVel.g = RandomFloat(mEndColorLow.g, mEndColorHigh.g);
        colVel.b = RandomFloat(mEndColorLow.b, mEndColorHigh.b);
        colVel.a = RandomFloat(mEndColorLow.a, mEndColorHigh.a);
        colVel.r -= pParticle->mCol.r;
        colVel.g -= pParticle->mCol.g;
        colVel.b -= pParticle->mCol.b;
        colVel.a -= pParticle->mCol.a;
        const float fScale = 1.0f / (pParticle->mDeathFrame - pParticle->mBirthFrame);
        colVel.r *= fScale;
        colVel.a *= fScale;
        colVel.g *= fScale;
        colVel.b *= fScale;
        mEmitAccumulator -= 1.0f;
    }
}

void RndParticleSys::RandomizeColorAndSize(Particle *pParticle) {
    pParticle->mCol.r = RandomFloat(mStartColorLow.r, mStartColorHigh.r);
    pParticle->mCol.g = RandomFloat(mStartColorLow.g, mStartColorHigh.g);
    pParticle->mCol.b = RandomFloat(mStartColorLow.b, mStartColorHigh.b);
    pParticle->mCol.a = RandomFloat(mStartColorLow.a, mStartColorHigh.a);
    pParticle->mSize = RandomFloat(mSize.x, mSize.y);
}

void RndParticleSys::FreeAllParticles() {
    Particle *pParticle = mLiveParticles;
    while (pParticle != nullptr) {
        pParticle = FreeParticle(pParticle);
    }
}

void RndParticleSys::UpdateParticles(float fDelta) {
    if (fDelta == 0.0f) {
        return;
    }
    Vector3 force;
    force.x = mForce.x * fDelta;
    force.z = mForce.z * fDelta;
    force.y = mForce.y * fDelta;
    Particle *pParticle = mLiveParticles;
    while (pParticle != nullptr) {
        if (pParticle->mDeathFrame <= mFrame || mFrame < pParticle->mBirthFrame) {
            pParticle = FreeParticle(pParticle);
            continue;
        }
        pParticle->mPos.x += pParticle->mVel.x * fDelta;
        pParticle->mPos.y += pParticle->mVel.y * fDelta;
        pParticle->mPos.z += pParticle->mVel.z * fDelta;
        if (mBubble != 0) {
            const float fSin =
                FastSin((mFrame * pParticle->mBubbleFrequency) + pParticle->mBubblePhase + kHalfPi);
            const float fStep = fSin * pParticle->mBubbleFrequency * fDelta;
            pParticle->mPos.x += pParticle->mBubbleSize.x * fStep;
            pParticle->mPos.y += pParticle->mBubbleSize.y * fStep;
            pParticle->mPos.z += pParticle->mBubbleSize.z * fStep;
        }
        if (mCollide != 0) {
            const Plane &plane = mCollidePlane;
            const float fDistance = (plane.a * pParticle->mPos.x) + (plane.b * pParticle->mPos.y) +
                                    (plane.c * pParticle->mPos.z) + plane.d;
            if (!(0.0f < fDistance)) {
                const float fDot = (pParticle->mVel.x * plane.a) + (pParticle->mVel.y * plane.b) +
                                   (pParticle->mVel.z * plane.c);
                if (fDot < 0.0f) {
                    const float fTwice = fDot + fDot;
                    pParticle->mVel.x -= plane.a * fTwice;
                    pParticle->mVel.y -= plane.b * fTwice;
                    pParticle->mVel.z -= plane.c * fTwice;
                }
            }
        }
        pParticle->mVel.x += force.x;
        pParticle->mVel.y += force.y;
        pParticle->mVel.z += force.z;
        pParticle->mCol.r += pParticle->mColVel.r * fDelta;
        pParticle->mCol.g += pParticle->mColVel.g * fDelta;
        pParticle->mCol.b += pParticle->mColVel.b * fDelta;
        pParticle->mCol.a += pParticle->mColVel.a * fDelta;
        pParticle = pParticle->mNext;
    }
}

void RndParticleSys::SetMat(RndMat *pMat) {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    mMat = pMat;
    if (pMat != nullptr) {
        pMat->AddRef(this);
    }
}

int RndParticleSys::NumLiveParticles() const {
    int nCount = 0;
    for (const Particle *pParticle = mLiveParticles; pParticle != nullptr;
         pParticle = pParticle->mNext) {
        ++nCount;
    }
    return nCount;
}

void RndParticleSys::ListDrawObjects(std::list<RndObject *> &objects) {
    objects.push_back(this);
    objects.push_back(mMat);
    if (mMat == nullptr) {
        return;
    }
    for (int i = static_cast<int>(mMat->mStages.size()) - 1; i >= 0; --i) {
        objects.push_back(mMat->mStages[i].mTex);
    }
}

void RndParticleSys::ListDrawables(std::list<RndDrawable *> &drawables) {
    const RndCam *pCam = RndCam::sCurrent;
    for (const Particle *pParticle = mLiveParticles; pParticle != nullptr;
         pParticle = pParticle->mNext) {
        if (IsPointInsideFrustum(pParticle->mPos, pCam->mWorldFrustum)) {
            drawables.push_back(this);
            break;
        }
    }
    RndDrawable::ListDrawables(drawables);
}

PrnStream &operator<<(PrnStream &stream, const Particle &particle) {
    stream << "\n\tpos:" << particle.mPos;
    stream << "\n\tvel:" << particle.mVel;
    stream << "\n\tcol:" << particle.mCol;
    stream << "\n\tcolVel:" << particle.mColVel;
    stream << "size:" << particle.mSize << " deathFrame:" << particle.mDeathFrame
           << " birthFrame:" << particle.mBirthFrame;
    return stream;
}

PrnStream &operator<<(PrnStream &stream, RndParticleSys::Mode eMode) {
    if (eMode == RndParticleSys::kModePoint) {
        stream << "Point";
    } else if (eMode == RndParticleSys::kModeLine) {
        stream << "Line";
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, std::list<Plane> &planes) {
    int nCount;
    stream.ReadEndian(&nCount, sizeof(nCount));
    planes.resize(nCount);
    for (Plane &plane : planes) {
        Vector3 point;
        ReadFloat(stream, point.x);
        ReadFloat(stream, point.y);
        ReadFloat(stream, point.z);
        ReadFloat(stream, plane.a);
        ReadFloat(stream, plane.b);
        ReadFloat(stream, plane.c);
        plane.d = -((point.x * plane.a) + (point.y * plane.b) + (point.z * plane.c));
    }
    return stream;
}
