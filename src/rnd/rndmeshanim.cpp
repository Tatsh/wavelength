#include "rnd/rndmeshanim.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

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

// The keys at a frame, or nothing for no keys.
template <typename T>
const Key<T> *
KeysAt(const std::vector<Key<T>> &keys, float fFrame, const Key<T> *&pNext, float &fRatio) {
    const Key<T> *pPrev;
    AtFrame(keys, fFrame, pPrev, pNext, fRatio);
    return pPrev;
}

// Blend a position for each vertex both lists have.
// NTSC-U/C: 0x00236428, PAL: 0x0023efa8
void InterpPoints(const std::vector<Vector3> &prev,
                  const std::vector<Vector3> &next,
                  float fRatio,
                  std::vector<RndMesh::Vert> &verts) {
    const unsigned int nCount = prev.size() < verts.size() ? prev.size() : verts.size();
    for (unsigned int i = 0; i < nCount; ++i) {
        Interp(prev[i], next[i], fRatio, verts[i].mPos);
    }
}

// Blend texture coordinates for each vertex both lists have.
// NTSC-U/C: 0x00236580, PAL: 0x0023f100
void InterpTexs(const std::vector<Vector2> &prev,
                const std::vector<Vector2> &next,
                float fRatio,
                std::vector<RndMesh::Vert> &verts) {
    const unsigned int nCount = prev.size() < verts.size() ? prev.size() : verts.size();
    for (unsigned int i = 0; i < nCount; ++i) {
        Interp(prev[i], next[i], fRatio, verts[i].mTex);
    }
}

// Blend a colour for each vertex both lists have, taking either key exactly at a position of 0 or
// 1.
// NTSC-U/C: 0x002366a0, PAL: 0x0023f220
void InterpColors(const std::vector<Color> &prev,
                  const std::vector<Color> &next,
                  float fRatio,
                  std::vector<RndMesh::Vert> &verts) {
    const unsigned int nCount = prev.size() < verts.size() ? prev.size() : verts.size();
    for (unsigned int i = 0; i < nCount; ++i) {
        if (fRatio == 0.0f) {
            verts[i].mColor = prev[i];
        } else if (fRatio == 1.0f) {
            verts[i].mColor = next[i];
        } else {
            Interp(prev[i], next[i], fRatio, verts[i].mColor);
        }
    }
}

} // namespace

const char *RndMeshAnim::sClassName = "MeshAnim";
int RndMeshAnim::sRev = 0;

RndMeshAnim::RndMeshAnim(const char *pszName) : RndObject(pszName) {
    mMesh = nullptr;
    mKeysOwner = this;
}

RndMeshAnim::~RndMeshAnim() {
    ReleaseRefs();
}

float RndMeshAnim::EndFrame() {
    const RndMeshAnim *pOwner = mKeysOwner;
    const float fPoints = LastFrame(pOwner->mVertPointsKeys);
    const float fTexs = LastFrame(pOwner->mVertTexsKeys);
    const float fColors = LastFrame(pOwner->mVertColorsKeys);
    const float fTexsColors = fTexs < fColors ? fColors : fTexs;
    return fPoints < fTexsColors ? fTexsColors : fPoints;
}

void RndMeshAnim::ListAnimObjects(std::list<RndObject *> &objects) {
    objects.push_back(mMesh);
    RndAnimatable::ListAnimObjects(objects);
}

int RndMeshAnim::SetFrameSelf(float fFrame) {
    if (mMesh == nullptr) {
        return 1;
    }
    int nSync = 0;
    if (!mKeysOwner->mVertPointsKeys.empty()) {
        std::vector<RndMesh::Vert> &verts = mMesh->mGeomOwner->mVerts;
        nSync |= RndMesh::kSyncPositions;
        const Key<std::vector<Vector3>> *pNext;
        float fRatio = 0.0f;
        const auto *pPrev = KeysAt(mKeysOwner->mVertPointsKeys, fFrame, pNext, fRatio);
        if (pPrev != nullptr) {
            InterpPoints(pPrev->value, pNext->value, fRatio, verts);
        }
    }
    if (!mKeysOwner->mVertTexsKeys.empty()) {
        std::vector<RndMesh::Vert> &verts = mMesh->mGeomOwner->mVerts;
        nSync |= RndMesh::kSyncTexCoords;
        const Key<std::vector<Vector2>> *pNext;
        float fRatio = 0.0f;
        const auto *pPrev = KeysAt(mKeysOwner->mVertTexsKeys, fFrame, pNext, fRatio);
        if (pPrev != nullptr) {
            InterpTexs(pPrev->value, pNext->value, fRatio, verts);
        }
    }
    if (!mKeysOwner->mVertColorsKeys.empty()) {
        std::vector<RndMesh::Vert> &verts = mMesh->mGeomOwner->mVerts;
        nSync |= RndMesh::kSyncColors;
        const Key<std::vector<Color>> *pNext;
        float fRatio = 0.0f;
        const auto *pPrev = KeysAt(mKeysOwner->mVertColorsKeys, fFrame, pNext, fRatio);
        if (pPrev != nullptr) {
            InterpColors(pPrev->value, pNext->value, fRatio, verts);
        }
    }
    if (nSync != 0) {
        mMesh->Sync(nSync);
    }
    return 1;
}

void RndMeshAnim::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndMeshAnim]\n";
    stream << "light:" << static_cast<const RndObject *>(mMesh)
           << " keysOwner:" << static_cast<const RndObject *>(mKeysOwner) << "\n";
    stream << "vertPointsKeys:" << mVertPointsKeys << "\n";
    stream << "vertTexsKeys:" << mVertTexsKeys << "\n";
    stream << "vertColorsKeys:" << mVertColorsKeys << "\n";
}

void RndMeshAnim::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    WriteName(stream, mMesh);
    stream << mVertPointsKeys << mVertTexsKeys << mVertColorsKeys;
    WriteName(stream, mKeysOwner);
}

void RndMeshAnim::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    if (static_cast<RndObject *>(mMesh) == pFrom) {
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
    if (static_cast<RndObject *>(mKeysOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mKeysOwner != nullptr) {
            mKeysOwner = dynamic_cast<RndMeshAnim *>(pOwner);
        }
        if (mKeysOwner != nullptr) {
            mKeysOwner->AddRef(this);
        }
    }
}

void RndMeshAnim::Copy(const RndObject *pSource, int nFlags) {
    const RndMeshAnim *pAnim =
        pSource != nullptr ? dynamic_cast<const RndMeshAnim *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    ReleaseRefs();
    mMesh = pAnim->mMesh;
    if ((nFlags & kCopyShareKeys) == 0 && pAnim->mKeysOwner == pAnim) {
        mKeysOwner = this;
        mVertPointsKeys = pAnim->mVertPointsKeys;
        mVertTexsKeys = pAnim->mVertTexsKeys;
        mVertColorsKeys = pAnim->mVertColorsKeys;
    } else {
        mKeysOwner = pAnim->mKeysOwner;
    }
    AcquireRefs();
}

void RndMeshAnim::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new MeshAnim");
        return;
    }
    RndAnimatable::Load(stream);
    ReleaseRefs();
    mMesh = FindByName<RndMesh>(stream);
    stream >> mVertPointsKeys >> mVertTexsKeys >> mVertColorsKeys;
    mKeysOwner = FindByName<RndMeshAnim>(stream);
    AcquireRefs();
}

void RndMeshAnim::SetMesh(RndMesh *pMesh) {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
    mMesh = pMesh;
    if (pMesh != nullptr) {
        pMesh->AddRef(this);
    }
}

void RndMeshAnim::ReleaseRefs() {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
    if (mKeysOwner != nullptr) {
        mKeysOwner->RemoveRef(this);
    }
}

void RndMeshAnim::AcquireRefs() {
    if (mMesh != nullptr) {
        mMesh->AddRef(this);
    }
    if (mKeysOwner != nullptr) {
        mKeysOwner->AddRef(this);
    }
}
