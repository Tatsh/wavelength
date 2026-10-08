#include "rnd/rndtransanim.h"

#include <cmath>

#include "math/transformops.h"
#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The version that stored legacy keys inline instead of the key lists, and the first versions with
// each later field.
constexpr int kRevInlineKeys = 2;
constexpr int kRevKeyLists = 3;
constexpr int kRevScale = 1;
constexpr int kRevFollowPath = 2;

// The sample count a spline path is measured with, spread over its keys, and the least number of
// samples a segment receives.
constexpr int kPathSamples = 399;
constexpr int kMinSegmentSamples = 8;

// The weights Tangent() gives the neighbouring keys at the ends of a path.
constexpr float kEndTangentNear = 1.5f;
constexpr float kEndTangentFar = 0.25f;

// The up direction a path-following animation without rotation keys keeps.
const Vector3 kPathUp{0.0f, 0.0f, 1.0f, 1.0f};

void Subtract(const Vector3 &a, const Vector3 &b, Vector3 &out) {
    out.x = a.x - b.x;
    out.y = a.y - b.y;
    out.z = a.z - b.z;
}

float Length(const Vector3 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// The tangent of a path of vector keys at a key, from its neighbours, with one-sided estimates at
// the ends.
// NTSC-U/C: 0x00246618, PAL: 0x0024f0b8
void Tangent(const std::vector<Key<Vector3>> &keys, int nIndex, Vector3 &out) {
    const int nSize = static_cast<int>(keys.size());
    if (nSize == 2) {
        Subtract(keys[1].value, keys[0].value, out);
        return;
    }
    Vector3 far;
    if (nIndex == 0) {
        Subtract(keys[1].value, keys[0].value, out);
        out.z *= kEndTangentNear;
        out.y *= kEndTangentNear;
        out.x *= kEndTangentNear;
        Subtract(keys[2].value, keys[0].value, far);
    } else if (nIndex == nSize - 1) {
        Subtract(keys[nIndex].value, keys[nSize - 2].value, out);
        out.z *= kEndTangentNear;
        out.x *= kEndTangentNear;
        out.y *= kEndTangentNear;
        Subtract(keys[nIndex].value, keys[nSize - 3].value, far);
    } else {
        Subtract(keys[nIndex + 1].value, keys[nIndex - 1].value, out);
        out.z *= 0.5f;
        out.x *= 0.5f;
        out.y *= 0.5f;
        return;
    }
    out.z -= far.z * kEndTangentFar;
    out.x -= far.x * kEndTangentFar;
    out.y -= far.y * kEndTangentFar;
}

// The derivative of the Hermite curve through two points with two tangents.
// NTSC-U/C: 0x00246860, PAL: 0x0024f300
void HermiteDerivative(const Vector3 &p0,
                       const Vector3 &t0,
                       const Vector3 &p1,
                       const Vector3 &t1,
                       float fT,
                       Vector3 &out) {
    const float fT2 = fT * fT;
    const float fH00 = fT2 * 6.0f - fT * 6.0f;
    const float fH10 = fT2 * 3.0f - fT * 4.0f + 1.0f;
    const float fH01 = fT2 * -6.0f + fT * 6.0f;
    const float fH11 = fT2 * 3.0f - (fT + fT);
    out.z = p0.z * fH00;
    out.x = p0.x * fH00;
    out.y = p0.y * fH00;
    out.z += t0.z * fH10;
    out.y += t0.y * fH10;
    out.x += t0.x * fH10;
    out.z += p1.z * fH01;
    out.y += p1.y * fH01;
    out.x += p1.x * fH01;
    out.z += t1.z * fH11;
    out.y += t1.y * fH11;
    out.x += t1.x * fH11;
}

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

void ReadFloat(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

void SkipFloats(BinStream &stream, int nCount) {
    for (int i = 0; i < nCount; ++i) {
        float fUnused;
        ReadFloat(stream, fUnused);
    }
}

int ReadFlag(BinStream &stream) {
    unsigned char nFlag;
    stream.Read(&nFlag, sizeof(nFlag));
    return nFlag != 0;
}

void WriteFlag(BinStream &stream, int bFlag) {
    const char nFlag = static_cast<char>(bFlag);
    stream.Write(&nFlag, sizeof(nFlag));
}

// The floats a legacy vector key stores after its value, and before its frame, that nothing
// reads.
constexpr int kLegacyVectorKeyUnused = 9;
// The floats a legacy rotation key stores after its value that nothing reads.
constexpr int kLegacyQuatKeyUnused = 11;

// Read the legacy inline form of a key list, a count and then keys with values nothing reads. A
// count of 0 leaves the list unchanged unless the version is the one with inline keys only.
template <typename T>
void ReadLegacyKeys(BinStream &stream, int nRev, std::vector<Key<T>> &keys, int nUnused) {
    int nCount;
    stream.ReadEndian(&nCount, sizeof(nCount));
    if (nRev != kRevInlineKeys && nCount == 0) {
        return;
    }
    keys.resize(nCount, Key<T>());
    for (Key<T> &key : keys) {
        ReadKeyValue(stream, key.value);
        SkipFloats(stream, nUnused);
        ReadFloat(stream, key.frame);
    }
}

} // namespace

// The unit's static initialiser at NTSC-U/C: 0x00245498, PAL: 0x0024df68, is empty, and its global
// constructor at NTSC-U/C: 0x002454a0, PAL: 0x0024df70, calls it.

const char *RndTransAnim::sClassName = "TransAnim";
int RndTransAnim::sRev = 3;

RndTransAnim::RndTransAnim(const char *pszName) : RndObject(pszName) {
    mTransInterp = kInterpSpline;
    mTrans = nullptr;
    mScaleInterp = kInterpLinear;
    mFramesOwner = this;
    mRepeatTrans = 0;
    mFollowPath = 0;
}

RndTransAnim::~RndTransAnim() {
    ReleaseRefs();
}

float RndTransAnim::EndFrame() {
    const RndTransAnim *pOwner = mFramesOwner;
    const float fTrans = LastFrame(pOwner->mTransKeys);
    const float fRot = LastFrame(pOwner->mRotKeys);
    const float fScale = LastFrame(pOwner->mScaleKeys);
    const float fRotScale = fRot < fScale ? fScale : fRot;
    return fTrans < fRotScale ? fRotScale : fTrans;
}

void RndTransAnim::ListAnimObjects(std::list<RndObject *> &objects) {
    objects.push_back(mTrans);
    RndAnimatable::ListAnimObjects(objects);
}

int RndTransAnim::SetFrameSelf(float fFrame) {
    if (mTrans != nullptr) {
        Transform xfm = mTrans->mLocalXfm;
        MakeTransform(fFrame, xfm, 0);
        mTrans->mLocalXfm.mBasisX = xfm.mBasisX;
        mTrans->mLocalXfm.mBasisY = xfm.mBasisY;
        mTrans->mLocalXfm.mBasisZ = xfm.mBasisZ;
        mTrans->mDirty = 1;
        mTrans->mLocalXfm.mTranslation = xfm.mTranslation;
    }
    return 1;
}

float RndTransAnim::StartFrame() {
    const RndTransAnim *pOwner = mFramesOwner;
    const float fTrans = FirstFrame(pOwner->mTransKeys);
    const float fRot = FirstFrame(pOwner->mRotKeys);
    const float fScale = FirstFrame(pOwner->mScaleKeys);
    const float fRotScale = fScale < fRot ? fScale : fRot;
    return fRotScale < fTrans ? fRotScale : fTrans;
}

void RndTransAnim::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    RndDrawable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndTransAnim]\n";
    stream << "trans:" << static_cast<const RndObject *>(mTrans)
           << " framesOwner:" << static_cast<const RndObject *>(mFramesOwner) << "\n";
    stream << "rotKeys:" << mRotKeys << "\n";
    stream << "transKeys:" << mTransKeys << "\n";
    stream << "scaleKeys:" << mScaleKeys << "\n";
    stream << "transInterp:" << static_cast<Interp>(mTransInterp)
           << " scaleInterp:" << static_cast<Interp>(mScaleInterp) << "\n";
    stream << "repeatTrans:" << (mRepeatTrans != 0) << " followPath:" << (mFollowPath != 0) << "\n";
}

void RndTransAnim::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    RndDrawable::Save(stream);
    const RndObject *pTrans = mTrans;
    stream.WriteString(pTrans != nullptr ? pTrans->mName.c_str() : "");
    stream << mRotKeys << mTransKeys;
    const RndObject *pOwner = mFramesOwner;
    stream.WriteString(pOwner != nullptr ? pOwner->mName.c_str() : "");
    stream.WriteEndian(&mTransInterp, sizeof(mTransInterp));
    WriteFlag(stream, mRepeatTrans);
    stream << mScaleKeys;
    stream.WriteEndian(&mScaleInterp, sizeof(mScaleInterp));
    WriteFlag(stream, mFollowPath);
}

void RndTransAnim::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    if (static_cast<RndObject *>(mTrans) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mTrans != nullptr) {
            mTrans = pTo != nullptr ? dynamic_cast<RndTransformable *>(pTo) : nullptr;
        }
        if (mTrans != nullptr) {
            mTrans->AddRef(this);
        }
    }
    if (static_cast<RndObject *>(mFramesOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mFramesOwner != nullptr) {
            mFramesOwner = dynamic_cast<RndTransAnim *>(pOwner);
        }
        if (mFramesOwner != nullptr) {
            mFramesOwner->AddRef(this);
        }
    }
}

void RndTransAnim::Copy(const RndObject *pSource, int nFlags) {
    const RndTransAnim *pAnim =
        pSource != nullptr ? dynamic_cast<const RndTransAnim *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    ReleaseRefs();
    mTrans = pAnim->mTrans;
    mTransInterp = pAnim->mTransInterp;
    mRepeatTrans = pAnim->mRepeatTrans;
    mScaleInterp = pAnim->mScaleInterp;
    mFollowPath = pAnim->mFollowPath;
    if (pAnim->mFramesOwner == pAnim && (nFlags & kCopyShareKeys) == 0) {
        mFramesOwner = this;
        mTransKeys = pAnim->mTransKeys;
        mRotKeys = pAnim->mRotKeys;
        mScaleKeys = pAnim->mScaleKeys;
    } else {
        mFramesOwner = pAnim->mFramesOwner;
    }
    AcquireRefs();
}

void RndTransAnim::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new TransAnim");
        return;
    }
    RndAnimatable::Load(stream);
    RndDrawable::Load(stream);
    ReleaseRefs();
    mTrans = FindByName<RndTransformable>(stream);
    if (nRev != kRevInlineKeys) {
        stream >> mRotKeys >> mTransKeys;
    }
    mFramesOwner = FindByName<RndTransAnim>(stream);
    if (nRev < kRevKeyLists) {
        ReadLegacyKeys(stream, nRev, mTransKeys, kLegacyVectorKeyUnused);
        ReadLegacyKeys(stream, nRev, mRotKeys, kLegacyQuatKeyUnused);
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused));
    }
    int nTransInterp;
    stream.ReadEndian(&nTransInterp, sizeof(nTransInterp));
    mTransInterp = nTransInterp;
    mRepeatTrans = ReadFlag(stream);
    if (nRev >= kRevScale) {
        if (nRev != kRevInlineKeys) {
            stream >> mScaleKeys;
        }
        if (nRev < kRevKeyLists) {
            ReadLegacyKeys(stream, nRev, mScaleKeys, kLegacyVectorKeyUnused);
        }
        int nScaleInterp;
        stream.ReadEndian(&nScaleInterp, sizeof(nScaleInterp));
        mScaleInterp = nScaleInterp;
    }
    if (nRev >= kRevFollowPath) {
        mFollowPath = ReadFlag(stream);
    } else {
        const RndTransAnim *pOwner = mFramesOwner;
        mFollowPath = pOwner->mRotKeys.empty() && pOwner->mTransKeys.size() >= 2;
    }
    AcquireRefs();
}

void RndTransAnim::SetTrans(RndTransformable *pTrans) {
    if (mTrans != nullptr) {
        mTrans->RemoveRef(this);
    }
    mTrans = pTrans;
    if (pTrans != nullptr) {
        pTrans->AddRef(this);
    }
}

void RndTransAnim::SetFramesOwner(RndTransAnim *pOwner) {
    if (mFramesOwner != nullptr) {
        mFramesOwner->RemoveRef(this);
    }
    mFramesOwner = pOwner;
    if (pOwner != nullptr) {
        pOwner->AddRef(this);
    }
}

void RndTransAnim::MakeTransform(float fFrame, Transform &xfm, int bWhole) {
    const RndTransAnim *pOwner = mFramesOwner;
    Vector3 tangent;
    if (!pOwner->mTransKeys.empty()) {
        Vector3 loopOffset;
        if (mRepeatTrans != 0) {
            const Key<Vector3> &first = pOwner->mTransKeys.front();
            const Key<Vector3> &last = pOwner->mTransKeys.back();
            const float fSpan = last.frame - first.frame;
            const auto nLoops = static_cast<int>(std::floor((fFrame - first.frame) / fSpan));
            const auto fLoops = static_cast<float>(nLoops);
            fFrame -= fLoops * fSpan;
            loopOffset.z = (last.value.z - first.value.z) * fLoops;
            loopOffset.x = (last.value.x - first.value.x) * fLoops;
            loopOffset.y = (last.value.y - first.value.y) * fLoops;
        }
        InterpVector(pOwner->mTransKeys,
                     mTransInterp,
                     xfm.mTranslation,
                     mFollowPath != 0 ? &tangent : nullptr,
                     fFrame);
        if (mRepeatTrans != 0) {
            xfm.mTranslation.x += loopOffset.x;
            xfm.mTranslation.y += loopOffset.y;
            xfm.mTranslation.z += loopOffset.z;
        }
    } else if (bWhole != 0) {
        xfm.mTranslation = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
    }
    if (!pOwner->mRotKeys.empty()) {
        const Key<Quat> *pPrev;
        const Key<Quat> *pNext;
        float fRatio;
        AtFrame(pOwner->mRotKeys, fFrame, pPrev, pNext, fRatio);
        Quat rotation;
        if (pPrev != nullptr) {
            QuatSlerp(pPrev->value, pNext->value, rotation, fRatio);
        }
        Rnd::MakeRotMatrix(rotation, &xfm.mBasisX.x);
    } else if (bWhole != 0) {
        xfm.mBasisX.y = 0.0f;
        xfm.mBasisX.x = 1.0f;
        xfm.mBasisX.z = 0.0f;
        xfm.mBasisY.x = 0.0f;
        xfm.mBasisY.z = 0.0f;
        xfm.mBasisY.y = 1.0f;
        xfm.mBasisZ.x = 0.0f;
        xfm.mBasisZ.z = 1.0f;
        xfm.mBasisZ.y = 0.0f;
    }
    if (mFollowPath != 0 && !mFramesOwner->mTransKeys.empty()) {
        if (!mFramesOwner->mRotKeys.empty()) {
            Mat33BuildOrthonormal(&tangent.x, &xfm.mBasisZ.x, &xfm.mBasisX.x);
        } else {
            Mat33BuildOrthonormal(&tangent.x, &kPathUp.x, &xfm.mBasisX.x);
        }
    }
    if (!mFramesOwner->mScaleKeys.empty()) {
        Vector3 scale;
        InterpVector(mFramesOwner->mScaleKeys, mScaleInterp, scale, nullptr, fFrame);
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
}

float RndTransAnim::PathLength(float fStart, float fEnd) {
    const std::vector<Key<Vector3>> &keys = mFramesOwner->mTransKeys;
    const int nSize = static_cast<int>(keys.size());
    if (nSize < 2 || fEnd <= fStart) {
        return 0.0f;
    }
    if (fStart < keys.front().frame) {
        fStart = keys.front().frame;
    }
    if (keys.back().frame < fEnd) {
        fEnd = keys.back().frame;
    }
    const int nSamples = kPathSamples / nSize;
    const float fStep =
        1.0f / static_cast<float>(nSamples < kMinSegmentSamples ? kMinSegmentSamples : nSamples);
    const Key<Vector3> *pPrev;
    const Key<Vector3> *pNext;
    float fRatio = 0.0f;
    int nIndex = AtFrame(keys, fStart, pPrev, pNext, fRatio);
    if (pPrev == pNext) {
        ++nIndex;
    }
    float fLength = 0.0f;
    while (nIndex < nSize && keys[nIndex].frame <= fEnd) {
        fLength += SegmentLength(nIndex - 1, nIndex, fStep, fRatio, 1.0f);
        fRatio = 0.0f;
        ++nIndex;
    }
    if (nIndex != nSize) {
        const float fPrevFrame = keys[nIndex - 1].frame;
        const float fSpan = keys[nIndex].frame - fPrevFrame;
        const float fLast = fSpan == 0.0f ? 1.0f : (fEnd - fPrevFrame) / fSpan;
        fLength += SegmentLength(nIndex - 1, nIndex, fStep, fRatio, fLast);
    }
    return fLength;
}

void RndTransAnim::ReleaseRefs() {
    if (mTrans != nullptr) {
        mTrans->RemoveRef(this);
    }
    if (mFramesOwner != nullptr) {
        mFramesOwner->RemoveRef(this);
    }
}

void RndTransAnim::AcquireRefs() {
    if (mTrans != nullptr) {
        mTrans->AddRef(this);
    }
    if (mFramesOwner != nullptr) {
        mFramesOwner->AddRef(this);
    }
}

float RndTransAnim::SegmentLength(int nFrom, int nTo, float fStep, float fStart, float fEnd) {
    const std::vector<Key<Vector3>> &keys = mFramesOwner->mTransKeys;
    const Vector3 &from = keys[nFrom].value;
    const Vector3 &to = keys[nTo].value;
    if (mTransInterp != kInterpSpline) {
        Vector3 delta;
        Subtract(to, from, delta);
        return Length(delta);
    }
    float fLength = 0.0f;
    Vector3 tangentFrom;
    Vector3 tangentTo;
    Tangent(keys, nFrom, tangentFrom);
    Tangent(keys, nTo, tangentTo);
    float fT = fStart;
    while (fT < fEnd) {
        Vector3 derivative;
        HermiteDerivative(from, tangentFrom, to, tangentTo, fT, derivative);
        fT += fStep;
        fLength += Length(derivative) * fStep;
    }
    return fLength;
}

void InterpVector(const std::vector<Key<Vector3>> &keys,
                  int nInterp,
                  Vector3 &out,
                  Vector3 *pTangent,
                  float fFrame) {
    const Key<Vector3> *pPrev;
    const Key<Vector3> *pNext;
    float fRatio = 0.0f;
    int nIndex = AtFrame(keys, fFrame, pPrev, pNext, fRatio);
    if (pPrev == nullptr) {
        return;
    }
    Vector3 tangentPrev;
    Vector3 tangentNext;
    if (pPrev == pNext) {
        out = pPrev->value;
    } else if (nInterp == RndTransAnim::kInterpSpline) {
        const float fT = fRatio;
        const float fT2 = fT * fT;
        const float fT3 = fT2 * fT;
        const float fThreeT2 = fT2 * 3.0f;
        const float fH00 = (fT3 + fT3) - fThreeT2 + 1.0f;
        out.z = pPrev->value.z * fH00;
        out.x = pPrev->value.x * fH00;
        out.y = pPrev->value.y * fH00;
        Tangent(keys, nIndex - 1, tangentPrev);
        const float fH10 = fT3 - (fT2 + fT2) + fT;
        out.z += tangentPrev.z * fH10;
        out.x += tangentPrev.x * fH10;
        out.y += tangentPrev.y * fH10;
        const float fH01 = fT3 * -2.0f + fThreeT2;
        out.z += pNext->value.z * fH01;
        out.x += pNext->value.x * fH01;
        out.y += pNext->value.y * fH01;
        Tangent(keys, nIndex, tangentNext);
        const float fH11 = fT3 - fT2;
        out.x += tangentNext.x * fH11;
        out.z += tangentNext.z * fH11;
        out.y += tangentNext.y * fH11;
    } else if (fRatio == 0.0f) {
        out = pPrev->value;
    } else if (fRatio == 1.0f) {
        out = pNext->value;
    } else {
        const float fInverse = 1.0f - fRatio;
        out = pNext->value;
        out.x = pNext->value.x * fRatio + pPrev->value.x * fInverse;
        out.y = pNext->value.y * fRatio + pPrev->value.y * fInverse;
        out.z = pNext->value.z * fRatio + pPrev->value.z * fInverse;
    }
    if (pTangent == nullptr) {
        return;
    }
    const int nSize = static_cast<int>(keys.size());
    if (nSize < 2) {
        pTangent->z = 0.0f;
        pTangent->y = 1.0f;
        pTangent->x = 0.0f;
    } else if (nInterp == RndTransAnim::kInterpSpline) {
        if (pPrev == pNext) {
            Tangent(keys, nIndex != nSize ? nIndex : nIndex - 1, *pTangent);
        } else {
            HermiteDerivative(
                pPrev->value, tangentPrev, pNext->value, tangentNext, fRatio, *pTangent);
        }
    } else if (pPrev == pNext) {
        if (nIndex == 0) {
            nIndex = 1;
        } else if (nIndex == nSize) {
            nIndex = nSize - 1;
        }
        Subtract(keys[nIndex].value, keys[nIndex - 1].value, *pTangent);
    } else {
        Subtract(pNext->value, pPrev->value, *pTangent);
    }
}

PrnStream &operator<<(PrnStream &stream, RndTransAnim::Interp eInterp) {
    switch (eInterp) {
    case RndTransAnim::kInterpLinear:
        return stream << "Linear";
    case RndTransAnim::kInterpSpline:
        return stream << "Spline";
    default:
        return stream;
    }
}
