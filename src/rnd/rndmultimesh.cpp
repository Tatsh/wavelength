#include "rnd/rndmultimesh.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// Move a mesh to a transform and recompute its world transform.
inline void PlaceMesh(RndMesh *pMesh, const Transform &xfm) {
    pMesh->mLocalXfm = xfm;
    pMesh->mDirty = 1;
    pMesh->UpdateWorldXfm(nullptr, 0);
}

// Move a mesh back to the transforms PlaceMesh() replaced. The saved world transform is placed as
// the local one, then the saved local transform is restored without recomputing.
inline void RestoreMesh(RndMesh *pMesh, const Transform &localXfm, const Transform &worldXfm) {
    PlaceMesh(pMesh, worldXfm);
    pMesh->mLocalXfm = localXfm;
    pMesh->mDirty = 1;
}

void WriteRow(BinStream &stream, const Vector3 &row) {
    stream.WriteEndian(&row.x, sizeof(row.x));
    stream.WriteEndian(&row.y, sizeof(row.y));
    stream.WriteEndian(&row.z, sizeof(row.z));
}

void ReadRow(BinStream &stream, Vector3 &row) {
    stream.ReadEndian(&row.x, sizeof(row.x));
    stream.ReadEndian(&row.y, sizeof(row.y));
    stream.ReadEndian(&row.z, sizeof(row.z));
}

} // namespace

const char *RndMultiMesh::sClassName = "MultiMesh";
int RndMultiMesh::sRev = 0;
int RndMultiMesh::sLoadRev;

RndMultiMesh::RndMultiMesh(const char *pszName) : RndObject(pszName) {
    mMesh = nullptr;
    AcquireRefs();
}

RndMultiMesh::~RndMultiMesh() {
    ReleaseRefs();
}

void RndMultiMesh::ListDrawObjects(std::list<RndObject *> &objects) {
    if (mMesh != nullptr) {
        mMesh->ListDrawObjects(objects);
    }
}

void RndMultiMesh::ListDrawables(std::list<RndDrawable *> &drawables) {
    if (mMesh != nullptr) {
        const Transform localXfm = mMesh->mLocalXfm;
        const Transform worldXfm = mMesh->mWorldXfm;
        std::list<RndDrawable *> meshDrawables;
        for (const Transform &xfm : mTransforms) {
            PlaceMesh(mMesh, xfm);
            mMesh->ListDrawables(meshDrawables);
            if (!meshDrawables.empty()) {
                drawables.push_back(this);
                break;
            }
        }
        RestoreMesh(mMesh, localXfm, worldXfm);
    }
    RndDrawable::ListDrawables(drawables);
}

int RndMultiMesh::DrawShowing() {
    if (mMesh == nullptr) {
        return 1;
    }
    const Transform localXfm = mMesh->mLocalXfm;
    const Transform worldXfm = mMesh->mWorldXfm;
    for (const Transform &xfm : mTransforms) {
        PlaceMesh(mMesh, xfm);
        mMesh->DrawShowing(); // Yes, the binary discards the mesh's result.
    }
    RestoreMesh(mMesh, localXfm, worldXfm);
    return 1;
}

void RndMultiMesh::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndDrawable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndMultiMesh]\n";
    stream << "mesh: " << static_cast<const RndObject *>(mMesh) << " transforms: " << mTransforms
           << "\n";
}

void RndMultiMesh::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndDrawable::Save(stream);
    const RndObject *pMesh = mMesh;
    stream.WriteString(pMesh != nullptr ? pMesh->mName.c_str() : "");
    stream << mTransforms;
}

void RndMultiMesh::Replace(RndObject *pFrom, RndObject *pTo) {
    RndDrawable::Replace(pFrom, pTo);
    if (mMesh != pFrom) {
        return;
    }
    if (pFrom != nullptr) {
        pFrom->RemoveRef(this);
    }
    if (mMesh != nullptr) {
        mMesh = pTo != nullptr ? dynamic_cast<RndMesh *>(pTo) : nullptr;
    }
    if (mMesh != nullptr) {
        mMesh->AddRef(this);
    }
}

void RndMultiMesh::Copy(const RndObject *pSource, int nFlags) {
    const RndMultiMesh *pMultiMesh =
        pSource != nullptr ? dynamic_cast<const RndMultiMesh *>(pSource) : nullptr;
    RndDrawable::Copy(pSource, nFlags);
    ReleaseRefs();
    mMesh = pMultiMesh->mMesh;
    mTransforms = pMultiMesh->mTransforms;
    AcquireRefs();
}

void RndMultiMesh::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new MultiMesh");
        return;
    }
    RndDrawable::Load(stream);
    ReleaseRefs();
    String meshName;
    stream >> meshName;
    if (meshName.mLength == 0) {
        mMesh = nullptr;
    } else {
        RndObject *pObject = TheManager.Find(meshName.c_str());
        mMesh = pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr;
    }
    stream >> mTransforms;
    AcquireRefs();
}

void RndMultiMesh::SetMesh(RndMesh *pMesh) {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
    mMesh = pMesh;
    if (pMesh != nullptr) {
        pMesh->AddRef(this);
    }
}

void RndMultiMesh::AcquireRefs() {
    if (mMesh != nullptr) {
        mMesh->AddRef(this);
    }
}

void RndMultiMesh::ReleaseRefs() {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
}

BinStream &operator<<(BinStream &stream, const std::list<Transform> &transforms) {
    const int nSize = static_cast<int>(transforms.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const Transform &xfm : transforms) {
        WriteRow(stream, xfm.mBasisX);
        WriteRow(stream, xfm.mBasisY);
        WriteRow(stream, xfm.mBasisZ);
        WriteRow(stream, xfm.mTranslation);
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, std::list<Transform> &transforms) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    transforms.resize(nSize);
    for (Transform &xfm : transforms) {
        ReadRow(stream, xfm.mBasisX);
        ReadRow(stream, xfm.mBasisY);
        ReadRow(stream, xfm.mBasisZ);
        ReadRow(stream, xfm.mTranslation);
    }
    return stream;
}
