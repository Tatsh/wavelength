#include "rnd/rndgenerator.h"

#include <cmath>

#include "math/rand.h"
#include "math/transform.h"
#include "math/transformops.h"
#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The value mNextFrame holds before the first frame.
constexpr float kUnsetFrame = -9999999.0f;

// The frame SetPath() replaces with the start or the end of the path.
constexpr float kPathFrameDefault = -1.0f;

constexpr float kPi = 3.1415927f;
constexpr float kDegreesPerHalfTurn = 180.0f;

// The defaults the constructor sets.
constexpr float kDefaultRateGen = 100.0f;

enum Axis {
    kAxisX = 0,
    kAxisY = 1,
    kAxisZ = 2,
};

// The first version with the bases, the generation and scale ranges, the squared birth distance,
// the view, the view animation flag, the path frames, and the multimesh and the particle system.
// Earlier versions have a child flag that is no longer supported.
constexpr int kRevGenRanges = 1;
constexpr int kRevBases = 2;
constexpr int kRevSquareDist = 3;
constexpr int kRevView = 4;
constexpr int kRevAnimateFromStart = 5;
constexpr int kRevPathFrames = 6;
constexpr int kRevSubjects = 7;

constexpr int kDumpInstances = 2;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.GetLength() == 0) {
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

void WriteObjectName(BinStream &stream, const RndObject *pObject) {
    stream.WriteString(pObject != nullptr ? pObject->GetName() : "");
}

template <typename T>
void ReplaceRef(RndObject *pOwner, T *&pRef, RndObject *pFrom, RndObject *pTo) {
    if (static_cast<RndObject *>(pRef) != pFrom) {
        return;
    }
    if (pFrom != nullptr) {
        pFrom->RemoveRef(pOwner);
    }
    if (pRef != nullptr) {
        pRef = pTo != nullptr ? dynamic_cast<T *>(pTo) : nullptr;
    }
    if (pRef != nullptr) {
        pRef->AddRef(pOwner);
    }
}

// Scale the basis rows of a transform by the scale of each axis.
inline void ScaleRows(Transform &xfm, const Vector3 &scale) {
    xfm.mBasisX.x *= scale.x;
    xfm.mBasisX.y *= scale.x;
    xfm.mBasisX.z *= scale.x;
    xfm.mBasisY.x *= scale.y;
    xfm.mBasisY.y *= scale.y;
    xfm.mBasisY.z *= scale.y;
    xfm.mBasisZ.x *= scale.z;
    xfm.mBasisZ.y *= scale.z;
    xfm.mBasisZ.z *= scale.z;
}

} // namespace

const char *RndGenerator::sClassName = "Generator";
int RndGenerator::sRev = 7;

RndGenerator::RndGenerator(const char *pszName) : RndObject(pszName) {
    mAnimateFromStart = 1;
    mNextFrame = kUnsetFrame;
    mRateGenHigh = kDefaultRateGen;
    mScaleGenHigh = 1.0f;
    mPath = nullptr;
    mPathStartFrame = 0.0f;
    mPathEndFrame = 0.0f;
    mMesh = nullptr;
    mView = nullptr;
    mMultiMesh = nullptr;
    mParticleSys = nullptr;
    mBirthFrontOnly = 0;
    mBirthCheckDist = 0;
    mBirthSquareDist = 0.0f;
    mBirthCam = nullptr;
    mRateGenLow = kDefaultRateGen;
    mScaleGenLow = 1.0f;
    mPathVarMax[kAxisX] = 0.0f;
    mPathVarMax[kAxisY] = 0.0f;
    mPathVarMax[kAxisZ] = 0.0f;
}

RndGenerator::~RndGenerator() {
    ReleaseRefs();
}

void RndGenerator::Replace(RndObject *pFrom, RndObject *pTo) {
    RndTransformable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    RndAnimatable::Replace(pFrom, pTo);
    ReplaceRef(this, mMesh, pFrom, pTo);
    ReplaceRef(this, mPath, pFrom, pTo);
    ReplaceRef(this, mBirthCam, pFrom, pTo);
    ReplaceRef(this, mView, pFrom, pTo);
    ReplaceRef(this, mMultiMesh, pFrom, pTo);
    ReplaceRef(this, mParticleSys, pFrom, pTo);
}

void RndGenerator::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndTransformable::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndAnimatable::DumpText(stream);
    if (stream.GetDumpLevel() <= 0) {
        return;
    }
    stream << "[RndGenerator]\n";
    stream << "path:" << static_cast<const RndObject *>(mPath)
           << " mesh:" << static_cast<const RndObject *>(mMesh) << "\n";
    // Yes, the binary writes no line break between the next two values.
    stream << " birthFrontOnly:" << (mBirthFrontOnly != 0) << "birthSquareDist:" << mBirthSquareDist
           << " birthCam:" << static_cast<const RndObject *>(mBirthCam) << "\n";
    stream << "rateGenLow:" << mRateGenLow << " rateGenHigh:" << mRateGenHigh << "\n";
    stream << "scaleGenLow:" << mScaleGenLow << " scaleGenHigh:" << mScaleGenHigh << "\n";
    stream << "pathVarMax:(" << mPathVarMax[kAxisX] << ", " << mPathVarMax[kAxisY] << ", "
           << mPathVarMax[kAxisZ] << ")\n";
    stream << "view:" << static_cast<const RndObject *>(mView)
           << " animateFromStart:" << (mAnimateFromStart != 0) << "\n";
    stream << "multiMesh:" << static_cast<const RndObject *>(mMultiMesh)
           << " particleSys:" << static_cast<const RndObject *>(mParticleSys) << "\n";
    if (stream.GetDumpLevel() < kDumpInstances) {
        return;
    }
    stream << "instances:" << mInstances << "\n";
    stream << "pathEndFrame:" << mPathEndFrame << " pathStartFrame:" << mPathStartFrame << "\n";
}

void RndGenerator::Copy(const RndObject *pSource, int nFlags) {
    const RndGenerator *pGen =
        pSource != nullptr ? dynamic_cast<const RndGenerator *>(pSource) : nullptr;
    RndTransformable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    RndAnimatable::Copy(pSource, nFlags);
    ReleaseRefs();
    mMesh = pGen->mMesh;
    mPath = pGen->mPath;
    mBirthFrontOnly = pGen->mBirthFrontOnly;
    mBirthSquareDist = pGen->mBirthSquareDist;
    mBirthCam = pGen->mBirthCam;
    mRateGenLow = pGen->mRateGenLow;
    mRateGenHigh = pGen->mRateGenHigh;
    mScaleGenLow = pGen->mScaleGenLow;
    mScaleGenHigh = pGen->mScaleGenHigh;
    mPathVarMax[kAxisX] = pGen->mPathVarMax[kAxisX];
    mPathVarMax[kAxisY] = pGen->mPathVarMax[kAxisY];
    mPathVarMax[kAxisZ] = pGen->mPathVarMax[kAxisZ];
    mView = pGen->mView;
    mAnimateFromStart = pGen->mAnimateFromStart;
    mPathEndFrame = pGen->mPathEndFrame;
    mPathStartFrame = pGen->mPathStartFrame;
    mMultiMesh = pGen->mMultiMesh;
    mParticleSys = pGen->mParticleSys;
    AcquireRefs();
}

void RndGenerator::ReleaseRefs() {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
    if (mPath != nullptr) {
        mPath->RemoveRef(this);
    }
    if (mBirthCam != nullptr) {
        mBirthCam->RemoveRef(this);
    }
    if (mView != nullptr) {
        mView->RemoveRef(this);
    }
    if (mMultiMesh != nullptr) {
        mMultiMesh->RemoveRef(this);
    }
    if (mParticleSys != nullptr) {
        mParticleSys->RemoveRef(this);
    }
    mInstances.clear();
}

void RndGenerator::AcquireRefs() {
    if (mMesh != nullptr) {
        mMesh->AddRef(this);
    }
    if (mPath != nullptr) {
        mPath->AddRef(this);
    }
    if (mBirthCam != nullptr) {
        mBirthCam->AddRef(this);
    }
    if (mView != nullptr) {
        mView->AddRef(this);
    }
    if (mMultiMesh != nullptr) {
        mMultiMesh->AddRef(this);
    }
    if (mParticleSys != nullptr) {
        mParticleSys->AddRef(this);
    }
    ResetParticles();
}

void RndGenerator::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndTransformable::Save(stream);
    RndDrawable::Save(stream);
    RndAnimatable::Save(stream);
    WriteObjectName(stream, mMesh);
    WriteObjectName(stream, mPath);
    WriteFlag(stream, mBirthFrontOnly);
    WriteFloat(stream, mBirthSquareDist);
    WriteObjectName(stream, mBirthCam);
    WriteFloat(stream, mRateGenLow);
    WriteFloat(stream, mRateGenHigh);
    WriteFloat(stream, mScaleGenLow);
    WriteFloat(stream, mScaleGenHigh);
    WriteFloat(stream, mPathVarMax[kAxisX]);
    WriteFloat(stream, mPathVarMax[kAxisY]);
    WriteFloat(stream, mPathVarMax[kAxisZ]);
    WriteObjectName(stream, mView);
    WriteFlag(stream, mAnimateFromStart);
    WriteFloat(stream, mPathEndFrame);
    WriteFloat(stream, mPathStartFrame);
    WriteObjectName(stream, mMultiMesh);
    WriteObjectName(stream, mParticleSys);
}

void RndGenerator::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Generator");
        return;
    }
    if (nRev >= kRevBases) {
        RndTransformable::Load(stream);
        RndDrawable::Load(stream);
        RndAnimatable::Load(stream);
    }
    ReleaseRefs();
    mMesh = FindByName<RndMesh>(stream);
    mPath = FindByName<RndTransAnim>(stream);
    if (nRev < kRevSubjects) {
        // Yes, the binary reports the child flag only when the flag is clear.
        if (ReadFlag(stream) == 0) {
            DebugNotify("%s no longer supports childOfGen", mName.c_str());
        }
    }
    if (nRev < kRevGenRanges) {
        stream.ReadEndian(&mRateGenHigh, sizeof(mRateGenHigh));
        stream.ReadEndian(&mScaleGenHigh, sizeof(mScaleGenHigh));
    }
    mBirthFrontOnly = ReadFlag(stream);
    stream.ReadEndian(&mBirthSquareDist, sizeof(mBirthSquareDist));
    mBirthCam = FindByName<RndCam>(stream);
    if (nRev >= kRevGenRanges) {
        stream.ReadEndian(&mRateGenLow, sizeof(mRateGenLow));
        stream.ReadEndian(&mRateGenHigh, sizeof(mRateGenHigh));
        stream.ReadEndian(&mScaleGenLow, sizeof(mScaleGenLow));
        stream.ReadEndian(&mScaleGenHigh, sizeof(mScaleGenHigh));
        stream.ReadEndian(&mPathVarMax[kAxisX], sizeof(mPathVarMax[kAxisX]));
        stream.ReadEndian(&mPathVarMax[kAxisY], sizeof(mPathVarMax[kAxisY]));
        stream.ReadEndian(&mPathVarMax[kAxisZ], sizeof(mPathVarMax[kAxisZ]));
    } else {
        mRateGenLow = mRateGenHigh;
        mScaleGenLow = mScaleGenHigh;
        mPathVarMax[kAxisX] = 0.0f;
        mPathVarMax[kAxisZ] = 0.0f;
        mPathVarMax[kAxisY] = 0.0f;
    }
    if (nRev < kRevSquareDist) {
        mBirthSquareDist *= mBirthSquareDist;
    } else if (nRev < kRevView) {
        String name;
        stream >> name;
        if (name.GetLength() != 0) {
            TheManager.Find(name.c_str()); // Yes, the binary discards the object it finds.
        }
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused)); // Yes, the binary discards this word.
    }
    if (nRev >= kRevView) {
        mView = FindByName<RndView>(stream);
    }
    if (nRev >= kRevAnimateFromStart) {
        mAnimateFromStart = ReadFlag(stream);
    }
    if (nRev >= kRevPathFrames) {
        stream.ReadEndian(&mPathEndFrame, sizeof(mPathEndFrame));
        stream.ReadEndian(&mPathStartFrame, sizeof(mPathStartFrame));
    } else {
        if (mPath != nullptr) {
            mPathEndFrame = mPath->EndFrame();
        }
        mPathStartFrame = 0.0f;
    }
    if (nRev >= kRevSubjects) {
        mMultiMesh = FindByName<RndMultiMesh>(stream);
        mParticleSys = FindByName<RndParticleSys>(stream);
    }
    AcquireRefs();
}

void RndGenerator::ResetParticles() {
    if (mParticleSys == nullptr) {
        return;
    }
    mParticleSys->FreeAllParticles();
    for (unsigned int i = 0; i < mInstances.size(); ++i) {
        mCurParticle = mParticleSys->AllocParticle();
        if (mCurParticle != nullptr) {
            mParticleSys->RandomizeColorAndSize(mCurParticle);
        }
    }
}

void RndGenerator::SetPath(RndTransAnim *pPath, float fStart, float fEnd) {
    if (mPath != nullptr) {
        mPath->RemoveRef(this);
    }
    mPath = pPath;
    if (pPath != nullptr) {
        pPath->AddRef(this);
    }
    if (mPath != nullptr && fStart == kPathFrameDefault) {
        mPathStartFrame = mPath->StartFrame();
    } else {
        mPathStartFrame = fStart;
    }
    if (mPath != nullptr && fEnd == kPathFrameDefault) {
        mPathEndFrame = mPath->EndFrame();
    } else {
        mPathEndFrame = fEnd;
    }
}

void RndGenerator::Generate(float fFrame) {
    if (mBirthCam != nullptr) {
        const Vector3 &pos = mWorldXfm.mTranslation;
        const Vector3 &camPos = mBirthCam->GetWorldXfm().mTranslation;
        const float fDeltaX = pos.x - camPos.x;
        const float fDeltaY = pos.y - camPos.y;
        const float fDeltaZ = pos.z - camPos.z;
        if (mBirthFrontOnly != 0) {
            const Vector3 &forward = mBirthCam->GetWorldXfm().mBasisY;
            if ((fDeltaX * forward.x) + (fDeltaY * forward.y) + (fDeltaZ * forward.z) < 0.0f) {
                return;
            }
        }
        if (mBirthCheckDist != 0 &&
            mBirthSquareDist < (fDeltaX * fDeltaX) + (fDeltaY * fDeltaY) + (fDeltaZ * fDeltaZ)) {
            return;
        }
    }

    Instance instance;
    instance.mFrame = fFrame;
    instance.mXfm.mBasisX.x = 1.0f;
    instance.mXfm.mBasisX.y = 0.0f;
    instance.mXfm.mBasisX.z = 0.0f;
    instance.mXfm.mBasisY.x = 0.0f;
    instance.mXfm.mBasisY.y = 1.0f;
    instance.mXfm.mBasisY.z = 0.0f;
    instance.mXfm.mBasisZ.x = 0.0f;
    instance.mXfm.mBasisZ.y = 0.0f;
    instance.mXfm.mBasisZ.z = 1.0f;
    instance.mXfm.mTranslation.x = 0.0f;
    instance.mXfm.mTranslation.y = 0.0f;
    instance.mXfm.mTranslation.z = 0.0f;

    float angles[3] = {};
    if (0.0f < mPathVarMax[kAxisX]) {
        angles[kAxisX] = RandomFloat(-mPathVarMax[kAxisX], mPathVarMax[kAxisX]);
    }
    if (0.0f < mPathVarMax[kAxisY]) {
        angles[kAxisY] = RandomFloat(-mPathVarMax[kAxisY], mPathVarMax[kAxisY]);
    }
    if (0.0f < mPathVarMax[kAxisZ]) {
        angles[kAxisZ] = RandomFloat(-mPathVarMax[kAxisZ], mPathVarMax[kAxisZ]);
    }
    angles[kAxisZ] = (angles[kAxisZ] * kPi) / kDegreesPerHalfTurn;
    angles[kAxisX] = (angles[kAxisX] * kPi) / kDegreesPerHalfTurn;
    angles[kAxisY] = (angles[kAxisY] * kPi) / kDegreesPerHalfTurn;
    Rnd::MakeRotMatrix(angles, &instance.mXfm.mBasisX.x);
    Multiply(instance.mXfm, mWorldXfm, instance.mXfm);

    float fScale = mScaleGenLow;
    if (mScaleGenLow < mScaleGenHigh) {
        fScale = RandomFloat(mScaleGenLow, mScaleGenHigh);
    }
    instance.mScale.z = fScale;
    instance.mScale.x = fScale;
    instance.mScale.y = fScale;
    mInstances.push_back(instance);

    if (mParticleSys != nullptr) {
        mCurParticle = mParticleSys->AllocParticle();
        if (mCurParticle != nullptr) {
            mParticleSys->RandomizeColorAndSize(mCurParticle);
        }
    }
}

int RndGenerator::SetFrameSelf(float fFrame) {
    if (mNextFrame == kUnsetFrame) {
        mNextFrame = fFrame;
        return 1;
    }
    const int nDirection = 0.0f < mPathEndFrame - mPathStartFrame ? 1 : -1;
    mCurParticle = mParticleSys != nullptr ? mParticleSys->GetLiveParticles() : nullptr;
    const float fDirection = static_cast<float>(nDirection);
    for (auto it = mInstances.begin(); it != mInstances.end();) {
        const float fSpan = fDirection * (mPathEndFrame - mPathStartFrame);
        const float fAge = fFrame - it->mFrame;
        if (fSpan < fAge || fAge < 0.0f) {
            if (fAge < 0.0f) {
                mNextFrame = it->mFrame;
            }
            it = mInstances.erase(it);
            if (mCurParticle != nullptr) {
                mCurParticle = mParticleSys->FreeParticle(mCurParticle);
            }
        } else {
            ++it;
            if (mCurParticle != nullptr) {
                mCurParticle = mCurParticle->mNext;
            }
        }
    }

    if (mView != nullptr && mAnimateFromStart == 0) {
        mView->SetFrame(mFrame);
    }
    if (mRateGenLow < 0.0f) {
        return 1;
    }
    const float fEarliest = fFrame - std::fabs(mPathEndFrame - mPathStartFrame);
    if (mNextFrame < fEarliest) {
        mNextFrame = fEarliest;
    }
    const float fLatest = fFrame + mRateGenHigh;
    if (fLatest < mNextFrame) {
        mNextFrame = fLatest;
    }
    while (mNextFrame <= fFrame) {
        Generate(mNextFrame);
        mNextFrame = mNextFrame + RandomFloat(mRateGenLow, mRateGenHigh);
    }
    return 1;
}

void RndGenerator::ListDrawObjects(std::list<RndObject *> &objects) {
    objects.push_back(this);
    if (mMesh != nullptr) {
        mMesh->ListDrawObjects(objects);
    }
    if (mView != nullptr) {
        mView->ListDrawObjects(objects);
    }
    if (mMultiMesh != nullptr) {
        mMultiMesh->ListDrawObjects(objects);
    }
    if (mParticleSys != nullptr) {
        mParticleSys->ListDrawObjects(objects);
    }
}

float RndGenerator::InstanceXfm(const Instance &instance, float fDirection, Transform &xfm) {
    const float fAge = mFrame - instance.mFrame;
    mPath->MakeTransform((fAge * fDirection) + mPathStartFrame, xfm, 1);
    ScaleRows(xfm, instance.mScale);
    Multiply(xfm, instance.mXfm, xfm);
    return fAge;
}

void RndGenerator::ListDrawables(std::list<RndDrawable *> &drawables) {
    // Yes, the binary tests the instances only without a path or without a subject.
    if (mPath == nullptr || (mMesh == nullptr && mView == nullptr && mMultiMesh == nullptr &&
                             mParticleSys == nullptr)) {
        std::list<RndDrawable *> subjectDrawables;
        if (mMultiMesh != nullptr) {
            mMultiMesh->ListDrawables(subjectDrawables);
        } else if (mParticleSys != nullptr) {
            mParticleSys->ListDrawables(subjectDrawables);
        } else {
            const float fDirection = 0.0f < mPathEndFrame - mPathStartFrame ? 1.0f : -1.0f;
            for (const Instance &instance : mInstances) {
                Transform xfm;
                const float fAge = InstanceXfm(instance, fDirection, xfm);
                if (mMesh != nullptr) {
                    mMesh->GetLocalXfm() = xfm;
                    mMesh->SetDirty(1);
                    mMesh->UpdateWorldXfm(nullptr, 0);
                    mMesh->ListDrawables(subjectDrawables);
                } else if (mView != nullptr) {
                    mView->GetLocalXfm() = xfm;
                    mView->SetDirty(1);
                    if (mAnimateFromStart != 0) {
                        mView->SetFrame(fAge);
                    }
                    mView->UpdateWorldXfm(nullptr, 0);
                    mView->ListDrawables(subjectDrawables);
                }
                if (!subjectDrawables.empty()) {
                    break;
                }
            }
        }
        if (!subjectDrawables.empty()) {
            drawables.push_back(this);
        }
    }
    RndDrawable::ListDrawables(drawables);
}

void RndGenerator::DrawView(float fAge, const Transform &xfm) {
    mView->GetLocalXfm() = xfm;
    mView->SetDirty(1);
    if (mAnimateFromStart != 0) {
        mView->SetFrame(fAge);
    }
    mView->UpdateWorldXfm(nullptr, 0);
    mView->Draw();
}

void RndGenerator::DrawMesh([[maybe_unused]] float fAge, const Transform &xfm) {
    mMesh->GetLocalXfm() = xfm;
    mMesh->SetDirty(1);
    mMesh->UpdateWorldXfm(nullptr, 0);
    mMesh->Draw();
}

void RndGenerator::SetMultiMeshXfm([[maybe_unused]] float fAge, const Transform &xfm) {
    Transform &dest = *mCurXfm;
    ++mCurXfm;
    dest = xfm;
}

void RndGenerator::SetParticlePos([[maybe_unused]] float fAge, const Transform &xfm) {
    if (mCurParticle != nullptr) {
        mCurParticle->mPos = xfm.mTranslation;
        mCurParticle = mCurParticle->mNext;
    }
}

int RndGenerator::DrawShowing() {
    if (mPath == nullptr) {
        return 1;
    }
    if (mMesh == nullptr && mView == nullptr && mMultiMesh == nullptr && mParticleSys == nullptr) {
        return 1;
    }
    DrawInstanceFunc pfnDraw;
    if (mView != nullptr) {
        pfnDraw = &RndGenerator::DrawView;
    } else if (mMesh != nullptr) {
        pfnDraw = &RndGenerator::DrawMesh;
    } else if (mMultiMesh != nullptr) {
        if (mMultiMesh->GetTransforms().size() != mInstances.size()) {
            // The binary leaves this stack transform uninitialised. Here Vector3 sets each w to 1.
            Transform fill;
            mMultiMesh->GetTransforms().resize(mInstances.size(), fill);
        }
        mCurXfm = mMultiMesh->GetTransforms().begin();
        pfnDraw = &RndGenerator::SetMultiMeshXfm;
    } else {
        mCurParticle = mParticleSys->GetLiveParticles();
        pfnDraw = &RndGenerator::SetParticlePos;
    }

    const float fDirection = 0.0f < mPathEndFrame - mPathStartFrame ? 1.0f : -1.0f;
    for (const Instance &instance : mInstances) {
        Transform xfm;
        const float fAge = InstanceXfm(instance, fDirection, xfm);
        (this->*pfnDraw)(fAge, xfm);
    }
    if (mMultiMesh != nullptr) {
        mMultiMesh->Draw();
    } else if (mParticleSys != nullptr) {
        mParticleSys->Draw();
    }
    return 1;
}

PrnStream &operator<<(PrnStream &stream, const RndGenerator::Instance &instance) {
    stream << "(frameOrg: " << instance.mFrame << " xfmMod:" << instance.mXfm << ")";
    return stream;
}
