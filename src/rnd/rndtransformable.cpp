#include "rnd/rndtransformable.h"

#include <algorithm>
#include <cmath>

#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"

namespace {

// Load() maps the billboard index of a version before 3 to its mode.
// NTSC-U/C: 0x004105c0
constexpr int kOldBillboardModes[] = {
    RndTransformable::kBillboardNone,
    RndTransformable::kBillboardX,
    RndTransformable::kBillboardY,
    RndTransformable::kBillboardZ,
    RndTransformable::kBillboardXZ,
    RndTransformable::kBillboardXYZ,
};

// The bound Load() checks the old index against, which exceeds the six modes.
constexpr unsigned int kOldBillboardIndexLimit = 24;

// The first version that stores the billboard mode itself rather than an index.
constexpr int kRevBillboardMode = 3;
// Versions 2 through 4 store one byte after the origin that nothing reads.
constexpr int kRevFirstUnusedByte = 2;
constexpr int kRevLastUnusedByte = 4;

// The scratch transform BillboardXfm() reports.
// NTSC-U/C: 0x0043c7e0
Transform sBillboardXfm;

// A point transformed by the basis rows and the translation of a transform.
void TransformPoint(const Transform &xfm, const Vector3 &v, Vector3 &out) {
    const float fX =
        xfm.mBasisX.x * v.x + xfm.mBasisY.x * v.y + xfm.mBasisZ.x * v.z + xfm.mTranslation.x;
    const float fY =
        xfm.mBasisX.y * v.x + xfm.mBasisY.y * v.y + xfm.mBasisZ.y * v.z + xfm.mTranslation.y;
    const float fZ =
        xfm.mBasisX.z * v.x + xfm.mBasisY.z * v.y + xfm.mBasisZ.z * v.z + xfm.mTranslation.z;
    out.x = fX;
    out.y = fY;
    out.z = fZ;
}

void Cross(const Vector3 &a, const Vector3 &b, Vector3 &out) {
    const float fX = a.y * b.z - a.z * b.y;
    const float fY = a.z * b.x - a.x * b.z;
    const float fZ = a.x * b.y - a.y * b.x;
    out.x = fX;
    out.y = fY;
    out.z = fZ;
}

void Normalize(Vector3 &v) {
    const float fScale = 1.0f / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x *= fScale;
    v.y *= fScale;
    v.z *= fScale;
}

void ScaleRow(Vector3 &row, const Vector3 &source, float fScale) {
    row.x = source.x * fScale;
    row.y = source.y * fScale;
    row.z = source.z * fScale;
}

void Difference(const Vector3 &a, const Vector3 &b, Vector3 &out) {
    out.x = a.x - b.x;
    out.y = a.y - b.y;
    out.z = a.z - b.z;
}

void ReadFloat(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

void WriteFloat(BinStream &stream, float fValue) {
    stream.WriteEndian(&fValue, sizeof(fValue));
}

void ReadRow(BinStream &stream, Vector3 &row) {
    ReadFloat(stream, row.x);
    ReadFloat(stream, row.y);
    ReadFloat(stream, row.z);
}

void WriteRow(BinStream &stream, const Vector3 &row) {
    WriteFloat(stream, row.x);
    WriteFloat(stream, row.y);
    WriteFloat(stream, row.z);
}

void ReadXfm(BinStream &stream, Transform &xfm) {
    ReadRow(stream, xfm.mBasisX);
    ReadRow(stream, xfm.mBasisY);
    ReadRow(stream, xfm.mBasisZ);
    ReadRow(stream, xfm.mTranslation);
}

void WriteXfm(BinStream &stream, const Transform &xfm) {
    WriteRow(stream, xfm.mBasisX);
    WriteRow(stream, xfm.mBasisY);
    WriteRow(stream, xfm.mBasisZ);
    WriteRow(stream, xfm.mTranslation);
}

void SetIdentity(Transform &xfm) {
    xfm.mBasisX.x = 1.0f;
    xfm.mBasisX.z = 0.0f;
    xfm.mBasisX.y = 0.0f;
    xfm.mBasisY.x = 0.0f;
    xfm.mBasisY.z = 0.0f;
    xfm.mBasisY.y = 1.0f;
    xfm.mBasisZ.x = 0.0f;
    xfm.mBasisZ.z = 1.0f;
    xfm.mBasisZ.y = 0.0f;
    xfm.mTranslation = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
}

} // namespace

int RndTransformable::sRev = 5;

RndTransformable::RndTransformable() {
    mDirty = 1;
    mBillboard = kBillboardNone;
    SetIdentity(mLocalXfm);
    SetIdentity(mWorldXfm);
    mOrigin = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
}

RndTransformable::~RndTransformable() {
    ReleaseTransRefs();
}

void RndTransformable::SetBillboard(int nBillboard) {
    mBillboard = nBillboard;
    mDirty = 1;
}

int RndTransformable::UpdateWorldXfm(RndTransformable *pParent, int bForce) {
    if (bForce != 0 || mDirty != 0 || (pParent != nullptr && pParent->mDirty != 0)) {
        if (pParent == nullptr) {
            mWorldXfm = mLocalXfm;
        } else if (mBillboard == kBillboardLocalRotate) {
            TransformPoint(pParent->mWorldXfm, mLocalXfm.mTranslation, mWorldXfm.mTranslation);
            mWorldXfm.mBasisX = mLocalXfm.mBasisX;
            mWorldXfm.mBasisY = mLocalXfm.mBasisY;
            mWorldXfm.mBasisZ = mLocalXfm.mBasisZ;
        } else {
            Multiply(mWorldXfm, pParent->mWorldXfm, mLocalXfm);
        }
        if (mBillboard == kBillboardNone) {
            const Vector3 negated{-mOrigin.x, -mOrigin.y, -mOrigin.z};
            TransformPoint(mWorldXfm, negated, mWorldXfm.mTranslation);
        }
        mDirty = 1;
    }
    for (RndTransformable *pTrans : mTransList) {
        pTrans->UpdateWorldXfm(this, 0);
    }
    const int bUpdated = mDirty;
    mDirty = 0;
    return bUpdated;
}

void RndTransformable::DumpText(PrnStream &stream) {
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndTransformable]\n";
    stream << "localXfm:" << mLocalXfm << "\n";
    stream << "worldXfm:" << mWorldXfm << "\n";
    stream << "transList:" << mTransList << "\n";
    stream << "billboard:" << static_cast<Billboard>(mBillboard) << " origin:" << mOrigin << "\n";
}

void RndTransformable::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    WriteXfm(stream, mLocalXfm);
    WriteXfm(stream, mWorldXfm);
    stream << mTransList;
    stream.WriteEndian(&mBillboard, sizeof(mBillboard));
    WriteRow(stream, mOrigin);
}

void RndTransformable::Replace(RndObject *pFrom, RndObject *pTo) {
    for (auto it = mTransList.begin(); it != mTransList.end();) {
        RndObject *pChild = *it;
        if (pChild == pTo) {
            DebugNotify("%s already in %s", pTo->mName.c_str(), mName.c_str());
        }
        if (pChild == pFrom) {
            if (pFrom != nullptr) {
                pFrom->RemoveRef(this);
            }
            // Yes, the binary leaves a null entry unchanged even when it matches.
            if (*it != nullptr) {
                *it = pTo != nullptr ? dynamic_cast<RndTransformable *>(pTo) : nullptr;
            }
            if (*it != nullptr) {
                (*it)->AddRef(this);
            }
        }
        if (*it == nullptr) {
            it = mTransList.erase(it);
        } else {
            ++it;
        }
    }
}

void RndTransformable::Copy(const RndObject *pSource, int nFlags) {
    const RndTransformable *pTrans =
        pSource != nullptr ? dynamic_cast<const RndTransformable *>(pSource) : nullptr;
    ReleaseTransRefs();
    mLocalXfm = pTrans->mLocalXfm;
    mWorldXfm = pTrans->mWorldXfm;
    mBillboard = pTrans->mBillboard;
    mOrigin = pTrans->mOrigin;
    if ((nFlags & kCopyChildLists) != 0) {
        mTransList = pTrans->mTransList;
    }
    AcquireTransRefs();
}

void RndTransformable::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugWarn("Can't load new Transformable");
    }
    ReleaseTransRefs();
    ReadXfm(stream, mLocalXfm);
    ReadXfm(stream, mWorldXfm);
    stream >> mTransList;
    if (nRev > 0) {
        if (nRev < kRevBillboardMode) {
            unsigned int nIndex;
            stream.ReadEndian(&nIndex, sizeof(nIndex));
            // Yes, the binary bounds the index by 24 although the table has six modes.
            mBillboard = nIndex < kOldBillboardIndexLimit ? kOldBillboardModes[nIndex] : 0;
        } else {
            stream.ReadEndian(&mBillboard, sizeof(mBillboard));
        }
        ReadRow(stream, mOrigin);
    }
    if (nRev >= kRevFirstUnusedByte && nRev <= kRevLastUnusedByte) {
        char unused;
        stream.Read(&unused, sizeof(unused));
    }
    AcquireTransRefs();
}

void RndTransformable::SetOrigin(const Vector3 &origin) {
    mDirty = 1;
    mOrigin = origin;
}

RndTransformable *RndTransformable::Parent() {
    for (RndObject *pRef : mRefs) {
        RndTransformable *pTrans =
            pRef != nullptr ? dynamic_cast<RndTransformable *>(pRef) : nullptr;
        if (pTrans != nullptr &&
            std::find(pTrans->mTransList.begin(), pTrans->mTransList.end(), this) !=
                pTrans->mTransList.end()) {
            return pTrans;
        }
    }
    return nullptr;
}

bool RndTransformable::AddTrans(RndTransformable *pTrans) {
    if (std::find(mTransList.begin(), mTransList.end(), pTrans) != mTransList.end()) {
        DebugNotify("%s already in %s", pTrans->mName.c_str(), mName.c_str());
        return false;
    }
    if (pTrans != nullptr) {
        pTrans->AddRef(this);
    }
    mTransList.push_back(pTrans);
    pTrans->mDirty = 1;
    return true;
}

void RndTransformable::RemoveTrans(RndTransformable *pTrans) {
    if (std::find(mTransList.begin(), mTransList.end(), pTrans) == mTransList.end()) {
        return;
    }
    if (pTrans != nullptr) {
        pTrans->RemoveRef(this);
    }
    mTransList.remove(pTrans);
}

void RndTransformable::ClearTrans() {
    for (auto it = mTransList.begin(); it != mTransList.end();) {
        if (*it != nullptr) {
            (*it)->RemoveRef(this);
        }
        it = mTransList.erase(it);
    }
}

const Transform &RndTransformable::BillboardXfm() {
    if (mBillboard == kBillboardNone) {
        return mWorldXfm;
    }
    const Transform &cam = RndCam::sCurrent->mWorldXfm;
    Vector3 scale;
    if ((mBillboard & kBillboardScale) != 0) {
        MakeScale(mWorldXfm, scale);
    }
    if ((mBillboard & kBillboardSimpleXYZ) != 0) {
        sBillboardXfm.mTranslation = mWorldXfm.mTranslation;
    } else if ((mBillboard & kBillboardScale) != 0) {
        // The binary leaves the inverse unset when a scale is zero.
        Vector3 inverse{0.0f, 0.0f, 0.0f};
        if (scale.x != 0.0f && scale.y != 0.0f && scale.z != 0.0f) {
            inverse.z = 1.0f / scale.z;
            inverse.x = 1.0f / scale.x;
            inverse.y = 1.0f / scale.y;
        }
        ScaleRow(sBillboardXfm.mBasisX, mWorldXfm.mBasisX, inverse.x);
        ScaleRow(sBillboardXfm.mBasisY, mWorldXfm.mBasisY, inverse.y);
        ScaleRow(sBillboardXfm.mBasisZ, mWorldXfm.mBasisZ, inverse.z);
        sBillboardXfm.mTranslation = mWorldXfm.mTranslation;
    } else {
        sBillboardXfm = mWorldXfm;
    }

    Transform &xfm = sBillboardXfm;
    Vector3 toObject;
    switch (mBillboard & ~kBillboardScale) {
    case kBillboardSimpleXYZ:
        xfm.mBasisX = cam.mBasisX;
        xfm.mBasisY = cam.mBasisY;
        xfm.mBasisZ = cam.mBasisZ;
        break;
    case kBillboardXYZ:
        Difference(xfm.mTranslation, cam.mTranslation, xfm.mBasisY);
        xfm.mBasisZ = cam.mBasisZ;
        Normalize(xfm.mBasisY);
        Cross(xfm.mBasisY, xfm.mBasisZ, xfm.mBasisX);
        Normalize(xfm.mBasisX);
        Cross(xfm.mBasisX, xfm.mBasisY, xfm.mBasisZ);
        break;
    case kBillboardZ:
        Difference(xfm.mTranslation, cam.mTranslation, xfm.mBasisY);
        Cross(xfm.mBasisY, xfm.mBasisZ, toObject);
        xfm.mBasisX = toObject;
        Normalize(xfm.mBasisX);
        Cross(xfm.mBasisZ, xfm.mBasisX, xfm.mBasisY);
        break;
    case kBillboardX:
        Difference(xfm.mTranslation, cam.mTranslation, xfm.mBasisY);
        Cross(xfm.mBasisX, xfm.mBasisY, toObject);
        xfm.mBasisZ = toObject;
        Normalize(xfm.mBasisZ);
        Cross(xfm.mBasisZ, xfm.mBasisX, xfm.mBasisY);
        break;
    case kBillboardY:
        Difference(xfm.mTranslation, cam.mTranslation, toObject);
        Cross(cam.mBasisX, toObject, xfm.mBasisZ);
        Cross(xfm.mBasisY, xfm.mBasisZ, toObject);
        xfm.mBasisX = toObject;
        Normalize(xfm.mBasisX);
        Cross(xfm.mBasisX, xfm.mBasisY, xfm.mBasisZ);
        break;
    case kBillboardXZ:
        Difference(xfm.mTranslation, cam.mTranslation, toObject);
        xfm.mBasisY = toObject;
        Normalize(xfm.mBasisY);
        Cross(xfm.mBasisY, xfm.mBasisZ, toObject);
        xfm.mBasisX = toObject;
        Normalize(xfm.mBasisX);
        Cross(xfm.mBasisX, xfm.mBasisY, xfm.mBasisZ);
        break;
    default:
        break;
    }

    if ((mBillboard & kBillboardScale) != 0) {
        ScaleRow(xfm.mBasisX, xfm.mBasisX, scale.x);
        ScaleRow(xfm.mBasisY, xfm.mBasisY, scale.y);
        ScaleRow(xfm.mBasisZ, xfm.mBasisZ, scale.z);
    }
    const Vector3 negated{-mOrigin.x, -mOrigin.y, -mOrigin.z};
    TransformPoint(xfm, negated, xfm.mTranslation);
    return xfm;
}

void RndTransformable::ReleaseTransRefs() {
    for (RndTransformable *pTrans : mTransList) {
        if (pTrans != nullptr) {
            pTrans->RemoveRef(this);
        }
    }
}

void RndTransformable::AcquireTransRefs() {
    for (RndTransformable *pTrans : mTransList) {
        if (pTrans != nullptr) {
            pTrans->AddRef(this);
        }
    }
}

PrnStream &operator<<(PrnStream &stream, RndTransformable::Billboard eBillboard) {
    switch (eBillboard) {
    case RndTransformable::kBillboardNone:
        return stream << "None";
    case RndTransformable::kBillboardX:
        return stream << "X";
    case RndTransformable::kBillboardY:
        return stream << "Y";
    case RndTransformable::kBillboardZ:
        return stream << "Z";
    case RndTransformable::kBillboardXZ:
        return stream << "XZ";
    case RndTransformable::kBillboardXYZ:
        return stream << "XYZ";
    case RndTransformable::kBillboardSimpleXYZ:
        return stream << "SimpleXYZ";
    case RndTransformable::kBillboardLocalRotate:
        return stream << "LocalRotate";
    case RndTransformable::kBillboardScaleX:
        return stream << "ScaleX";
    case RndTransformable::kBillboardScaleY:
        return stream << "ScaleY";
    case RndTransformable::kBillboardScaleZ:
        return stream << "ScaleZ";
    case RndTransformable::kBillboardScaleXZ:
        return stream << "ScaleXZ";
    case RndTransformable::kBillboardScaleXYZ:
        return stream << "ScaleXYZ";
    case RndTransformable::kBillboardScaleSimpleXYZ:
        return stream << "ScaleSimpleXYZ";
    default:
        return stream;
    }
}
