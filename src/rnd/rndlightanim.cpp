#include "rnd/rndlightanim.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The first version without the two discarded colour key lists.
constexpr int kRevDiffuseOnly = 1;

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

} // namespace

const char *RndLightAnim::sClassName = "LightAnim";
int RndLightAnim::sRev = 1;

RndLightAnim::RndLightAnim(const char *pszName) : RndObject(pszName) {
    mLight = nullptr;
    mKeysOwner = this;
}

RndLightAnim::~RndLightAnim() {
    ReleaseRefs();
}

float RndLightAnim::EndFrame() {
    return LastFrame(mKeysOwner->mDiffuseKeys);
}

void RndLightAnim::ListAnimObjects(std::list<RndObject *> &objects) {
    objects.push_back(mLight);
    RndAnimatable::ListAnimObjects(objects);
}

int RndLightAnim::SetFrameSelf(float fFrame) {
    if (mLight == nullptr) {
        return 1;
    }
    Color color = mLight->mColor;
    InterpKeys(mKeysOwner->mDiffuseKeys, fFrame, color);
    mLight->SetColor(color);
    return 1;
}

void RndLightAnim::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndLightAnim]\n";
    stream << "light:" << static_cast<const RndObject *>(mLight)
           << " keysOwner:" << static_cast<const RndObject *>(mKeysOwner) << "\n";
    stream << "diffuseKeys:" << mDiffuseKeys << "\n";
}

void RndLightAnim::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    WriteName(stream, mLight);
    stream << mDiffuseKeys;
    WriteName(stream, mKeysOwner);
}

void RndLightAnim::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    if (static_cast<RndObject *>(mLight) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mLight != nullptr) {
            mLight = pTo != nullptr ? dynamic_cast<RndLight *>(pTo) : nullptr;
        }
        if (mLight != nullptr) {
            mLight->AddRef(this);
        }
    }
    if (static_cast<RndObject *>(mKeysOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mKeysOwner != nullptr) {
            mKeysOwner = dynamic_cast<RndLightAnim *>(pOwner);
        }
        if (mKeysOwner != nullptr) {
            mKeysOwner->AddRef(this);
        }
    }
}

void RndLightAnim::Copy(const RndObject *pSource, int nFlags) {
    const RndLightAnim *pAnim =
        pSource != nullptr ? dynamic_cast<const RndLightAnim *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    ReleaseRefs();
    mLight = pAnim->mLight;
    if ((nFlags & kCopyShareKeys) == 0 && pAnim->mKeysOwner == pAnim) {
        mKeysOwner = this;
        mDiffuseKeys = pAnim->mDiffuseKeys;
    } else {
        mKeysOwner = pAnim->mKeysOwner;
    }
    AcquireRefs();
}

void RndLightAnim::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new LightAnim");
        return;
    }
    RndAnimatable::Load(stream);
    ReleaseRefs();
    mLight = FindByName<RndLight>(stream);
    if (nRev < kRevDiffuseOnly) {
        std::vector<Key<Color>> discarded;
        stream >> discarded;
    }
    stream >> mDiffuseKeys;
    if (nRev < kRevDiffuseOnly) {
        std::vector<Key<Color>> discarded;
        stream >> discarded;
    }
    mKeysOwner = FindByName<RndLightAnim>(stream);
    AcquireRefs();
}

void RndLightAnim::ReleaseRefs() {
    if (mLight != nullptr) {
        mLight->RemoveRef(this);
    }
    if (mKeysOwner != nullptr) {
        mKeysOwner->RemoveRef(this);
    }
}

void RndLightAnim::AcquireRefs() {
    if (mLight != nullptr) {
        mLight->AddRef(this);
    }
    if (mKeysOwner != nullptr) {
        mKeysOwner->AddRef(this);
    }
}
