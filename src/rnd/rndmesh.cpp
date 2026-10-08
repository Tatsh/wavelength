#include "rnd/rndmesh.h"

#include <cmath>

#include "math/frustum.h"
#include "math/triangle.h"
#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"

namespace {

// The first version without each legacy field, and the first version with each later field.
constexpr int kRevStripsFirst = 1;
constexpr int kRevStripsLast = 3;
constexpr int kRevNoBillboard = 3;
constexpr int kRevEdges = 5;
constexpr int kRevKeepEdgesFirst = 5;
constexpr int kRevKeepEdgesLast = 7;
constexpr int kRevLod = 6;
constexpr int kRevUnusedByte = 7;
constexpr int kRevUnusedIntFirst = 9;
constexpr int kRevUnusedIntLast = 10;
constexpr int kRevNoWeights = 10;
constexpr int kRevNoVertExtras = 11;
constexpr int kRevMutable = 12;
constexpr int kRevNoFaceOwner = 13;
constexpr int kRevBones = 14;

// The floats the first version stores after its fields, a legacy vertex stores after its texture
// coordinates, and the first version stores after each face.
constexpr int kFirstRevUnusedFloats = 4;
constexpr int kVertUnusedFloats = 2;
constexpr int kFaceUnusedFloats = 3;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

void WriteName(BinStream &stream, const RndObject *pObject) {
    stream.WriteString(pObject != nullptr ? pObject->mName.c_str() : "");
}

void ReadFloat(BinStream &stream, float &fValue) {
    stream.ReadEndian(&fValue, sizeof(fValue));
}

void WriteFloat(BinStream &stream, float fValue) {
    stream.WriteEndian(&fValue, sizeof(fValue));
}

void SkipFloats(BinStream &stream, int nCount) {
    for (int i = 0; i < nCount; ++i) {
        float fUnused;
        ReadFloat(stream, fUnused);
    }
}

void ReadVector(BinStream &stream, Vector3 &v) {
    ReadFloat(stream, v.x);
    ReadFloat(stream, v.y);
    ReadFloat(stream, v.z);
}

void WriteVector(BinStream &stream, const Vector3 &v) {
    WriteFloat(stream, v.x);
    WriteFloat(stream, v.y);
    WriteFloat(stream, v.z);
}

void ReadTransform(BinStream &stream, Transform &xfm) {
    ReadVector(stream, xfm.mBasisX);
    ReadVector(stream, xfm.mBasisY);
    ReadVector(stream, xfm.mBasisZ);
    ReadVector(stream, xfm.mTranslation);
}

void WriteTransform(BinStream &stream, const Transform &xfm) {
    WriteVector(stream, xfm.mBasisX);
    WriteVector(stream, xfm.mBasisY);
    WriteVector(stream, xfm.mBasisZ);
    WriteVector(stream, xfm.mTranslation);
}

unsigned char ReadByte(BinStream &stream) {
    unsigned char nByte;
    stream.Read(&nByte, sizeof(nByte));
    return nByte;
}

// Read the legacy triangle strips, a list of index lists that nothing uses.
void SkipStrips(BinStream &stream) {
    int nStrips;
    stream.ReadEndian(&nStrips, sizeof(nStrips));
    for (int i = 0; i < nStrips; ++i) {
        int nIndices;
        stream.ReadEndian(&nIndices, sizeof(nIndices));
        for (int j = 0; j < nIndices; ++j) {
            unsigned short nUnused;
            stream.ReadEndian(&nUnused, sizeof(nUnused));
        }
    }
}

void MultiplyPoint(const Transform &xfm, const Vector3 &v, Vector3 &out) {
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

void MultiplyVector(const Transform &xfm, const Vector3 &v, Vector3 &out) {
    const float fX = xfm.mBasisX.x * v.x + xfm.mBasisY.x * v.y + xfm.mBasisZ.x * v.z;
    const float fY = xfm.mBasisX.y * v.x + xfm.mBasisY.y * v.y + xfm.mBasisZ.y * v.z;
    const float fZ = xfm.mBasisX.z * v.x + xfm.mBasisY.z * v.y + xfm.mBasisZ.z * v.z;
    out.x = fX;
    out.y = fY;
    out.z = fZ;
}

// The inverse of a general transform, by the adjugate of the basis. A singular basis inverts to
// zero.
void InvertGeneral(const Transform &xfm, Transform &out) {
    const Vector3 &x = xfm.mBasisX;
    const Vector3 &y = xfm.mBasisY;
    const Vector3 &z = xfm.mBasisZ;
    const float fDeterminant = x.x * (y.y * z.z - z.y * y.z) - x.y * (y.x * z.z - z.x * y.z) +
                               x.z * (y.x * z.y - z.x * y.y);
    const float fInverse = fDeterminant == 0.0f ? 0.0f : 1.0f / fDeterminant;
    out.mBasisX.x = (y.y * z.z - y.z * z.y) * fInverse;
    out.mBasisX.y = -(x.y * z.z - z.y * x.z) * fInverse;
    out.mBasisX.z = (x.y * y.z - y.y * x.z) * fInverse;
    out.mBasisY.x = -(y.x * z.z - y.z * z.x) * fInverse;
    out.mBasisY.y = (x.x * z.z - z.x * x.z) * fInverse;
    out.mBasisY.z = -(x.x * y.z - y.x * x.z) * fInverse;
    out.mBasisZ.x = (y.x * z.y - z.x * y.y) * fInverse;
    out.mBasisZ.y = -(x.x * z.y - z.x * x.y) * fInverse;
    out.mBasisZ.z = (x.x * y.y - x.y * y.x) * fInverse;
    const Vector3 negated{-xfm.mTranslation.x, -xfm.mTranslation.y, -xfm.mTranslation.z, 1.0f};
    MultiplyVector(out, negated, out.mTranslation);
}

void Subtract(const Vector3 &a, const Vector3 &b, Vector3 &out) {
    out.x = a.x - b.x;
    out.y = a.y - b.y;
    out.z = a.z - b.z;
}

void Cross(const Vector3 &a, const Vector3 &b, Vector3 &out) {
    out.x = a.y * b.z - a.z * b.y;
    out.y = a.z * b.x - a.x * b.z;
    out.z = a.x * b.y - a.y * b.x;
}

} // namespace

const char *RndMesh::sClassName = "Mesh";
int RndMesh::sRev = 14;
int RndMesh::sLoadRev;

RndMesh::RndMesh(const char *pszName) : RndObject(pszName) {
    mZMode = kZModeZReadWrite;
    mZFunc = kZFuncLess;
    mTransOwner = this;
    mMat = nullptr;
    mGeomOwner = this;
    mBones = nullptr;
    mMinScreen = 0.0f;
    mNext = nullptr;
    mMutable = 0;
    mSphere.mCenter = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
    mSphere.mRadius = 0.0f;
}

RndMesh::~RndMesh() {
    ReleaseRefs();
}

void RndMesh::ListDrawObjects(std::list<RndObject *> &objects) {
    objects.push_back(this);
    objects.push_back(mMat);
    objects.push_back(mGeomOwner);
    objects.push_back(mTransOwner);
    if (mMat != nullptr) {
        for (int i = static_cast<int>(mMat->mStages.size()) - 1; i >= 0; --i) {
            objects.push_back(mMat->mStages[i].mTex);
        }
    }
    if (mBones != nullptr) {
        objects.push_back(mBones->mTrans[0]);
        objects.push_back(mBones->mTrans[1]);
    }
}

void RndMesh::ListDrawables(std::list<RndDrawable *> &drawables) {
    if (mSphere.mRadius != 0.0f) {
        Sphere worldSphere;
        MultiplyPoint(mTransOwner->mWorldXfm, mSphere.mCenter, worldSphere.mCenter);
        worldSphere.mRadius = mSphere.mRadius;
        if (IsSphereOutsideFrustum(worldSphere, RndCam::sCurrent->mWorldFrustum) != 0) {
            return;
        }
    }
    drawables.push_back(this);
    RndDrawable::ListDrawables(drawables);
}

void RndMesh::Collide(const Segment &segment, std::list<Collision> &collisions) {
    const Transform &world = mTransOwner->mWorldXfm;
    Sphere worldSphere;
    MultiplyPoint(world, mSphere.mCenter, worldSphere.mCenter);
    worldSphere.mRadius = mSphere.mRadius;
    if (mShowing == 0) {
        return;
    }
    if (mSphere.mRadius != 0.0f) {
        float fT;
        if (!Intersect(segment, worldSphere, fT)) {
            return;
        }
    }
    Transform inverse;
    InvertGeneral(world, inverse);
    Segment local;
    MultiplyPoint(inverse, segment.mEnds[0], local.mEnds[0]);
    MultiplyPoint(inverse, segment.mEnds[1], local.mEnds[1]);
    const int nCull = mMat != nullptr ? mMat->mCull : RndMat::kCullCw;
    for (const Face &face : mGeomOwner->mFaces) {
        const std::vector<Vert> &verts = mGeomOwner->mVerts;
        Triangle triangle;
        triangle.mOrigin = verts[face.mVerts[0]].mPos;
        Subtract(verts[face.mVerts[1]].mPos, triangle.mOrigin, triangle.mEdge1);
        Subtract(verts[face.mVerts[2]].mPos, triangle.mOrigin, triangle.mEdge2);
        Cross(triangle.mEdge1, triangle.mEdge2, triangle.mNormal);
        float fT;
        if (Intersect(local, triangle, nCull, fT)) {
            collisions.push_back(Collision{this, fT});
        }
    }
    RndCollideable::Collide(segment, collisions);
}

void RndMesh::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndTransformable::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndCollideable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndMesh]\n";
    stream << "zMode:" << static_cast<ZMode>(mZMode) << " zFunc:" << static_cast<ZFunc>(mZFunc)
           << " mat:" << static_cast<const RndObject *>(mMat) << "\n";
    stream << "geomOwner:" << static_cast<const RndObject *>(mGeomOwner) << "\n";
    const RndTransformable *pTrans1 = mBones != nullptr ? mBones->mTrans[0] : nullptr;
    stream << "transOwner:" << static_cast<const RndObject *>(mTransOwner)
           << " boneTrans1:" << static_cast<const RndObject *>(pTrans1) << "\n";
    const RndTransformable *pTrans2 = mBones != nullptr ? mBones->mTrans[1] : nullptr;
    stream << "boneTrans2:" << static_cast<const RndObject *>(pTrans2) << " sphere:" << mSphere
           << "\n";
    stream << "next:" << static_cast<const RndObject *>(mNext) << " minScreen:" << mMinScreen
           << "\n";
    stream << "mutable: " << (mMutable != 0) << "\n";
    stream << "verts:" << mVerts << "\n";
    stream << "faces:" << mFaces << "\n";
    stream << "edges:" << mEdges << "\n";
}

void RndMesh::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndTransformable::Save(stream);
    RndDrawable::Save(stream);
    RndCollideable::Save(stream);
    stream.WriteEndian(&mZMode, sizeof(mZMode));
    stream.WriteEndian(&mZFunc, sizeof(mZFunc));
    WriteName(stream, mMat);
    WriteName(stream, mGeomOwner);
    WriteName(stream, mTransOwner);
    WriteVector(stream, mSphere.mCenter);
    WriteFloat(stream, mSphere.mRadius);
    WriteName(stream, mNext);
    WriteFloat(stream, mMinScreen);
    const char nMutable = static_cast<char>(mMutable);
    stream.Write(&nMutable, sizeof(nMutable));
    stream << mVerts << mFaces << mEdges;
    if (mBones != nullptr) {
        WriteName(stream, mBones->mTrans[0]);
        WriteName(stream, mBones->mTrans[1]);
        WriteTransform(stream, mBones->mXfms[0]);
        WriteTransform(stream, mBones->mXfms[1]);
    } else {
        const int nNoBones = 0; // Yes, the binary writes a bare 0, which reads as an empty name.
        stream.WriteEndian(&nNoBones, sizeof(nNoBones));
    }
}

void RndMesh::Replace(RndObject *pFrom, RndObject *pTo) {
    RndTransformable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    RndCollideable::Replace(pFrom, pTo);
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
    if (static_cast<RndObject *>(mNext) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mNext != nullptr) {
            mNext = pTo != nullptr ? dynamic_cast<RndMesh *>(pTo) : nullptr;
        }
        if (mNext != nullptr) {
            mNext->AddRef(this);
        }
    }
    if (static_cast<RndObject *>(mGeomOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mGeomOwner != nullptr) {
            mGeomOwner = dynamic_cast<RndMesh *>(pOwner);
        }
        if (mGeomOwner != nullptr) {
            mGeomOwner->AddRef(this);
        }
    }
    if (static_cast<RndObject *>(mTransOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mTransOwner != nullptr) {
            mTransOwner = dynamic_cast<RndTransformable *>(pOwner);
        }
        if (mTransOwner != nullptr) {
            mTransOwner->AddRef(this);
        }
    }
    if (mBones == nullptr) {
        return;
    }
    for (RndTransformable *&pTrans : mBones->mTrans) {
        if (static_cast<RndObject *>(pTrans) == pFrom) {
            if (pFrom != nullptr) {
                pFrom->RemoveRef(this);
            }
            if (pTrans != nullptr) {
                pTrans = pTo != nullptr ? dynamic_cast<RndTransformable *>(pTo) : nullptr;
            }
            if (pTrans != nullptr) {
                pTrans->AddRef(this);
            }
        }
    }
    if (mBones->mTrans[0] == nullptr) {
        if (mBones->mTrans[1] != nullptr) {
            mBones->mTrans[1]->RemoveRef(this);
        }
        delete mBones;
        mBones = nullptr;
    }
}

void RndMesh::Copy(const RndObject *pSource, int nFlags) {
    const RndMesh *pMesh = pSource != nullptr ? dynamic_cast<const RndMesh *>(pSource) : nullptr;
    RndTransformable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    RndCollideable::Copy(pSource, nFlags);
    ReleaseRefs();
    mZMode = pMesh->mZMode;
    mZFunc = pMesh->mZFunc;
    mMat = pMesh->mMat;
    mSphere = pMesh->mSphere;
    mNext = pMesh->mNext;
    mMutable = pMesh->mMutable;
    mMinScreen = pMesh->mMinScreen;
    if ((nFlags & kCopyShareGeometry) == 0 && pMesh->mGeomOwner == pMesh) {
        mGeomOwner = this;
        mVerts = pMesh->mVerts;
        mFaces = pMesh->mFaces;
        mEdges = pMesh->mEdges;
    } else {
        mGeomOwner = pMesh->mGeomOwner;
    }
    if ((nFlags & kCopyBones) != 0) {
        if (pMesh->mBones != nullptr) {
            mBones = new Bones;
            mBones->mTrans[0] = pMesh->mBones->mTrans[0];
            mBones->mTrans[1] = pMesh->mBones->mTrans[1];
            // Yes, the binary copies the first bind transform twice and leaves the second unset.
            mBones->mXfms[0] = pMesh->mBones->mXfms[0];
            mBones->mXfms[0] = pMesh->mBones->mXfms[0];
        }
        mTransOwner = pMesh->mTransOwner;
    } else if (pMesh->mTransOwner == static_cast<const RndTransformable *>(pMesh)) {
        mTransOwner = this;
    } else {
        mTransOwner = pMesh->mTransOwner;
    }
    AcquireRefs();
}

void RndMesh::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new Mesh");
        return;
    }
    RndTransformable::Load(stream);
    RndDrawable::Load(stream);
    RndCollideable::Load(stream);
    ReleaseRefs();
    int nZMode;
    stream.ReadEndian(&nZMode, sizeof(nZMode));
    int nZFunc;
    stream.ReadEndian(&nZFunc, sizeof(nZFunc));
    mZMode = nZMode;
    mZFunc = nZFunc;
    if (sLoadRev < kRevNoBillboard) {
        int nBillboard;
        stream.ReadEndian(&nBillboard, sizeof(nBillboard));
        SetBillboard(nBillboard);
    }
    mMat = FindByName<RndMat>(stream);
    mGeomOwner = FindByName<RndMesh>(stream);
    if (sLoadRev < kRevNoFaceOwner) {
        const RndMesh *pFaceOwner = FindByName<RndMesh>(stream);
        if (pFaceOwner != mGeomOwner) {
            DebugNotify("Combining face and vert owner of %s", mName.c_str());
        }
    }
    mTransOwner = FindByName<RndTransformable>(stream);
    if (sLoadRev < kRevBones) {
        FindByName<RndTransformable>(stream); // Yes, the binary discards the legacy bones.
        FindByName<RndTransformable>(stream);
    }
    if (sLoadRev < kRevNoBillboard) {
        Vector3 origin;
        ReadVector(stream, origin);
        SetOrigin(origin);
    }
    ReadVector(stream, mSphere.mCenter);
    ReadFloat(stream, mSphere.mRadius);
    bool bKeepEdges = true;
    if (sLoadRev >= kRevKeepEdgesFirst && sLoadRev <= kRevKeepEdgesLast) {
        bKeepEdges = ReadByte(stream) != 0;
    }
    if (sLoadRev >= kRevLod) {
        mNext = FindByName<RndMesh>(stream);
        ReadFloat(stream, mMinScreen);
    }
    if (sLoadRev >= kRevMutable) {
        mMutable = ReadByte(stream) != 0;
    }
    if (sLoadRev == kRevUnusedByte) {
        ReadByte(stream);
    }
    if (sLoadRev >= kRevUnusedIntFirst && sLoadRev <= kRevUnusedIntLast) {
        int nUnused;
        stream.ReadEndian(&nUnused, sizeof(nUnused));
    }
    stream >> mVerts >> mFaces;
    if (sLoadRev >= kRevEdges) {
        stream >> mEdges;
    }
    if (!bKeepEdges) {
        mEdges.clear();
    }
    if (sLoadRev >= kRevBones) {
        RndTransformable *pTrans1 = FindByName<RndTransformable>(stream);
        if (pTrans1 != nullptr) {
            mBones = new Bones;
            mBones->mTrans[0] = pTrans1;
            mBones->mTrans[1] = FindByName<RndTransformable>(stream);
            ReadTransform(stream, mBones->mXfms[0]);
            ReadTransform(stream, mBones->mXfms[1]);
        }
    }
    if (sLoadRev >= kRevStripsFirst && sLoadRev <= kRevStripsLast) {
        SkipStrips(stream);
    }
    if (sLoadRev == 0) {
        ReadByte(stream);
        SkipFloats(stream, kFirstRevUnusedFloats);
    }
    AcquireRefs();
}

void RndMesh::SetMat(RndMat *pMat) {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    mMat = pMat;
    if (pMat != nullptr) {
        pMat->AddRef(this);
    }
}

bool RndMesh::CheckLod(Sphere &worldSphere) {
    const RndCam *pCam = RndCam::sCurrent;
    if (mSphere.mRadius == 0.0f) {
        return true;
    }
    MultiplyPoint(mTransOwner->mWorldXfm, mSphere.mCenter, worldSphere.mCenter);
    worldSphere.mRadius = mSphere.mRadius;
    if (IsSphereOutsideFrustum(worldSphere, pCam->mWorldFrustum) != 0) {
        return false;
    }
    if (mMinScreen == 0.0f) {
        return true;
    }
    const Vector3 &forward = pCam->mOrthoWorldXfm.mBasisY;
    const Vector3 &center = worldSphere.mCenter;
    const float fDepth = center.x * forward.x + center.y * forward.y + center.z * forward.z +
                         pCam->mInvWorldXfm.mTranslation.y;
    const float fSize = worldSphere.mRadius * pCam->mLocalProject.mBasisX.x / std::fabs(fDepth) *
                        pCam->mScreenRect.w;
    if (mMinScreen <= fSize) {
        return true;
    }
    RndMesh *pLod = mNext;
    while (pLod != nullptr && fSize < pLod->mMinScreen) {
        pLod = pLod->mNext;
    }
    if (pLod != nullptr) {
        const float fRadius = pLod->mSphere.mRadius;
        pLod->Draw();
        pLod->mSphere.mRadius = fRadius; // Yes, the binary restores the radius after the draw.
    }
    return false;
}

void RndMesh::BoundingBox(Box &box) {
    const std::vector<Vert> &verts = mGeomOwner->mVerts;
    if (verts.empty()) {
        box.mMin = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
        box.mMax = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
        return;
    }
    box.mMin = verts.front().mPos;
    box.mMax = verts.front().mPos;
    for (auto it = verts.begin() + 1; it != verts.end(); ++it) {
        box.GrowToContain(it->mPos);
    }
}

void RndMesh::TransformVerts(const Transform &xfm) {
    for (Vert &vert : mGeomOwner->mVerts) {
        MultiplyPoint(xfm, vert.mPos, vert.mPos);
        MultiplyVector(xfm, vert.mNorm, vert.mNorm);
        const float fScale =
            1.0f / std::sqrt(vert.mNorm.x * vert.mNorm.x + vert.mNorm.y * vert.mNorm.y +
                             vert.mNorm.z * vert.mNorm.z);
        vert.mNorm.x *= fScale;
        vert.mNorm.y *= fScale;
        vert.mNorm.z *= fScale;
    }
    Sync(kSyncPositions | kSyncNormals);
}

void RndMesh::ReleaseRefs() {
    if (mNext != nullptr) {
        mNext->RemoveRef(this);
    }
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    if (mGeomOwner != nullptr) {
        mGeomOwner->RemoveRef(this);
    }
    if (mTransOwner != nullptr) {
        mTransOwner->RemoveRef(this);
    }
    if (mBones != nullptr) {
        if (mBones->mTrans[0] != nullptr) {
            mBones->mTrans[0]->RemoveRef(this);
        }
        if (mBones->mTrans[1] != nullptr) {
            mBones->mTrans[1]->RemoveRef(this);
        }
        delete mBones;
        mBones = nullptr;
    }
}

void RndMesh::AcquireRefs() {
    if (mNext != nullptr) {
        mNext->AddRef(this);
    }
    if (mMat != nullptr) {
        mMat->AddRef(this);
    }
    if (mGeomOwner != nullptr) {
        mGeomOwner->AddRef(this);
    }
    if (mTransOwner != nullptr) {
        mTransOwner->AddRef(this);
    }
    if (mBones != nullptr) {
        if (mBones->mTrans[0] != nullptr) {
            mBones->mTrans[0]->AddRef(this);
        }
        if (mBones->mTrans[1] != nullptr) {
            mBones->mTrans[1]->AddRef(this);
        }
    }
    Sync(kSyncAll);
}

PrnStream &operator<<(PrnStream &stream, const RndMesh::Vert &vert) {
    stream << "\n\tp:" << vert.mPos;
    stream << "\n\tw1:" << vert.mWeight1 << " w2:" << vert.mWeight2;
    stream << "\n\tn:" << vert.mNorm;
    stream << "\n\tc:" << vert.mColor;
    stream << "\n\tt:" << vert.mTex;
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const RndMesh::Face &face) {
    stream << "(v1:" << face.mVerts[0] << " v2:" << face.mVerts[1] << " v3:" << face.mVerts[2]
           << ")";
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const RndMesh::Edge &edge) {
    stream << "(v1:" << edge.mVerts[0] << " v2:" << edge.mVerts[1] << ")";
    return stream;
}

PrnStream &operator<<(PrnStream &stream, RndMesh::ZMode eMode) {
    switch (eMode) {
    case RndMesh::kZModeDisable:
        stream << "Disable";
        break;
    case RndMesh::kZModeZReadOnly:
        stream << "ZReadOnly";
        break;
    case RndMesh::kZModeZReadWrite:
        stream << "ZReadWrite";
        break;
    case RndMesh::kZModeWReadOnly:
        stream << "WReadOnly";
        break;
    case RndMesh::kZModeWReadWrite:
        stream << "WReadWrite";
        break;
    }
    return stream;
}

PrnStream &operator<<(PrnStream &stream, RndMesh::ZFunc eFunc) {
    switch (eFunc) {
    case RndMesh::kZFuncNever:
        stream << "Never";
        break;
    case RndMesh::kZFuncLess:
        stream << "Less";
        break;
    case RndMesh::kZFuncEqual:
        stream << "Equal";
        break;
    case RndMesh::kZFuncLessEqual:
        stream << "LessEqual";
        break;
    case RndMesh::kZFuncGreater:
        stream << "Greater";
        break;
    case RndMesh::kZFuncNotEqual:
        stream << "NotEqual";
        break;
    case RndMesh::kZFuncGreaterEqual:
        stream << "GreaterEqual";
        break;
    case RndMesh::kZFuncAlways:
        stream << "Always";
        break;
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const RndMesh::Vert &vert) {
    WriteVector(stream, vert.mPos);
    WriteFloat(stream, vert.mWeight1);
    WriteFloat(stream, vert.mWeight2);
    WriteVector(stream, vert.mNorm);
    WriteFloat(stream, vert.mColor.r);
    WriteFloat(stream, vert.mColor.g);
    WriteFloat(stream, vert.mColor.b);
    WriteFloat(stream, vert.mColor.a);
    WriteFloat(stream, vert.mTex.x);
    WriteFloat(stream, vert.mTex.y);
    return stream;
}

BinStream &operator>>(BinStream &stream, RndMesh::Vert &vert) {
    ReadVector(stream, vert.mPos);
    if (RndMesh::sLoadRev != kRevNoWeights) {
        ReadFloat(stream, vert.mWeight1);
        ReadFloat(stream, vert.mWeight2);
    }
    ReadVector(stream, vert.mNorm);
    ReadFloat(stream, vert.mColor.r);
    ReadFloat(stream, vert.mColor.g);
    ReadFloat(stream, vert.mColor.b);
    ReadFloat(stream, vert.mColor.a);
    ReadFloat(stream, vert.mTex.x);
    ReadFloat(stream, vert.mTex.y);
    if (RndMesh::sLoadRev < kRevNoVertExtras) {
        SkipFloats(stream, kVertUnusedFloats);
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const RndMesh::Face &face) {
    for (const unsigned short nVert : face.mVerts) {
        stream.WriteEndian(&nVert, sizeof(nVert));
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, RndMesh::Face &face) {
    for (unsigned short &nVert : face.mVerts) {
        stream.ReadEndian(&nVert, sizeof(nVert));
    }
    if (RndMesh::sLoadRev <= 0) {
        SkipFloats(stream, kFaceUnusedFloats);
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const RndMesh::Edge &edge) {
    for (const unsigned short nVert : edge.mVerts) {
        stream.WriteEndian(&nVert, sizeof(nVert));
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, RndMesh::Edge &edge) {
    for (unsigned short &nVert : edge.mVerts) {
        stream.ReadEndian(&nVert, sizeof(nVert));
    }
    return stream;
}
