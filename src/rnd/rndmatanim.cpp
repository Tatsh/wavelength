#include "rnd/rndmatanim.h"

#include "math/transformops.h"
#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The first versions with stage keys before the texture keys, with texture keys, and without
// ambient keys.
constexpr int kRevStageKeys = 1;
constexpr int kRevTexKeys = 2;
constexpr int kRevNoAmbient = 3;

// The first version with colour keys.
constexpr int kRevColorKeys = 2;

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

const char *RndMatAnim::sClassName = "MatAnim";
int RndMatAnim::sRev = 3;
int RndMatAnim::sLoadRev;

void RndMatAnim::Stage::Print(PrnStream &stream) const {
    stream << " transKeys:" << mTransKeys;
    stream << " scaleKeys:" << mScaleKeys << " rotKeys:" << mRotKeys;
    stream << " texKeys:" << mTexKeys << " matAnim:" << static_cast<const RndObject *>(mMatAnim);
}

void RndMatAnim::Stage::Save(BinStream &stream) const {
    stream << mTransKeys << mScaleKeys << mRotKeys << mTexKeys;
}

void RndMatAnim::Stage::Load(BinStream &stream) {
    if (sLoadRev < kRevTexKeys) {
        std::list<RndTex *> texs;
        stream >> texs;
        mTexKeys.clear();
        float fFrame = 0.0f;
        for (RndTex *pTex : texs) {
            const Key<RndTex *> *pPrev;
            const Key<RndTex *> *pNext;
            float fRatio;
            const int nIndex = AtFrame(mTexKeys, fFrame, pPrev, pNext, fRatio);
            mTexKeys.insert(mTexKeys.begin() + nIndex, Key<RndTex *>{pTex, fFrame});
            fFrame += 1.0f;
        }
    }
    if (sLoadRev >= kRevStageKeys) {
        stream >> mTransKeys >> mScaleKeys >> mRotKeys;
    }
    if (sLoadRev >= kRevTexKeys) {
        stream >> mTexKeys;
    }
}

RndMatAnim::RndMatAnim(const char *pszName) : RndObject(pszName) {
    mMat = nullptr;
    mKeysOwner = this;
}

RndMatAnim::~RndMatAnim() {
    ReleaseRefs();
}

float RndMatAnim::EndFrame() {
    const RndMatAnim *pOwner = mKeysOwner;
    float fEnd = 0.0f;
    for (const Stage &stage : pOwner->mStages) {
        const float fTrans = LastFrame(stage.mTransKeys);
        const float fScale = LastFrame(stage.mScaleKeys);
        if (fEnd < (fTrans < fScale ? fScale : fTrans)) {
            fEnd = fTrans < fScale ? fScale : fTrans;
        }
        const float fRot = LastFrame(stage.mRotKeys);
        const float fTex = LastFrame(stage.mTexKeys);
        if (fEnd < (fRot < fTex ? fTex : fRot)) {
            fEnd = fRot < fTex ? fTex : fRot;
        }
    }
    const float fLight = LastFrame(pOwner->mLightKeys);
    const float fBase = LastFrame(pOwner->mBaseKeys);
    if (fEnd < (fLight < fBase ? fBase : fLight)) {
        fEnd = fLight < fBase ? fBase : fLight;
    }
    const float fAlpha = LastFrame(pOwner->mAlphaKeys);
    const float fEdge = LastFrame(pOwner->mEdgeKeys);
    if (fEnd < (fAlpha < fEdge ? fEdge : fAlpha)) {
        fEnd = fAlpha < fEdge ? fEdge : fAlpha;
    }
    return fEnd;
}

void RndMatAnim::ListAnimObjects(std::list<RndObject *> &objects) {
    objects.push_back(mMat);
    RndAnimatable::ListAnimObjects(objects);
}

int RndMatAnim::SetFrameSelf(float fFrame) {
    if (mMat == nullptr) {
        return 1;
    }
    const RndMatAnim *pOwner = mKeysOwner;
    std::vector<RndMat::Stage> &matStages = mMat->mStages;
    unsigned int nIndex = 0;
    for (auto it = pOwner->mStages.begin();
         it != pOwner->mStages.end() && nIndex < matStages.size();
         ++it, ++nIndex) {
        const Stage &stage = *it;
        RndMat::Stage &matStage = matStages[nIndex];
        InterpKeys(stage.mTransKeys, fFrame, matStage.mXfm.mTranslation);
        if (!stage.mRotKeys.empty()) {
            Vector3 angles;
            InterpKeys(stage.mRotKeys, fFrame, angles);
            Rnd::MakeRotMatrix(&angles.x, &matStage.mXfm.mBasisX.x);
        }
        if (!stage.mScaleKeys.empty()) {
            Vector3 scale;
            InterpKeys(stage.mScaleKeys, fFrame, scale);
            Vector3 &basisX = matStage.mXfm.mBasisX;
            basisX.x *= scale.x;
            basisX.y *= scale.x;
            basisX.z *= scale.x;
            Vector3 &basisY = matStage.mXfm.mBasisY;
            basisY.x *= scale.y;
            basisY.y *= scale.y;
            basisY.z *= scale.y;
            Vector3 &basisZ = matStage.mXfm.mBasisZ;
            basisZ.x *= scale.z;
            basisZ.y *= scale.z;
            basisZ.z *= scale.z;
        }
        if (!stage.mTexKeys.empty()) {
            RndTex *pTex = nullptr;
            InterpKeys(stage.mTexKeys, fFrame, pTex);
            matStage.SetTex(pTex);
        }
    }
    if (!pOwner->mLightKeys.empty()) {
        Color color;
        InterpKeys(pOwner->mLightKeys, fFrame, color);
        mMat->SetLightColor(color);
    }
    if (!pOwner->mBaseKeys.empty()) {
        Color color;
        InterpKeys(pOwner->mBaseKeys, fFrame, color);
        mMat->SetBaseColor(color);
    }
    if (!pOwner->mEdgeKeys.empty()) {
        Color color;
        InterpKeys(pOwner->mEdgeKeys, fFrame, color);
        mMat->SetEdgeColor(color);
    }
    if (!pOwner->mAlphaKeys.empty()) {
        float fAlpha;
        InterpKeys(pOwner->mAlphaKeys, fFrame, fAlpha);
        mMat->SetAlpha(fAlpha);
    }
    return 1;
}

void RndMatAnim::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndMatAnim]\n";
    stream << "mat:" << static_cast<const RndObject *>(mMat) << " stages:" << mStages << "\n";
    stream << "keysOwner:" << static_cast<const RndObject *>(mKeysOwner)
           << " lightKeys:" << mLightKeys << "\n";
    stream << "baseKeys:" << mBaseKeys << "\n";
    stream << "edgeKeys:" << mEdgeKeys << "\n";
    stream << "alphaKeys:" << mAlphaKeys << "\n";
}

void RndMatAnim::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    WriteName(stream, mMat);
    stream << mStages;
    WriteName(stream, mKeysOwner);
    stream << mLightKeys << mBaseKeys << mEdgeKeys << mAlphaKeys;
}

void RndMatAnim::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
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
    if (static_cast<RndObject *>(mKeysOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mKeysOwner != nullptr) {
            mKeysOwner = dynamic_cast<RndMatAnim *>(pOwner);
        }
        if (mKeysOwner != nullptr) {
            mKeysOwner->AddRef(this);
        }
    }
    for (Stage &stage : mStages) {
        std::vector<Key<RndTex *>> &keys = stage.mTexKeys;
        auto it = keys.begin();
        while (it != keys.end()) {
            if (static_cast<RndObject *>(it->value) == pFrom) {
                if (pFrom != nullptr) {
                    pFrom->RemoveRef(this);
                }
                if (it->value != nullptr) {
                    it->value = pTo != nullptr ? dynamic_cast<RndTex *>(pTo) : nullptr;
                    if (it->value != nullptr) {
                        it->value->AddRef(this);
                    }
                }
            }
            if (it->value == nullptr) {
                it = keys.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void RndMatAnim::Copy(const RndObject *pSource, int nFlags) {
    const RndMatAnim *pAnim =
        pSource != nullptr ? dynamic_cast<const RndMatAnim *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    ReleaseRefs();
    mMat = pAnim->mMat;
    if ((nFlags & kCopyShallow) == 0 && pAnim->mKeysOwner == pAnim) {
        mKeysOwner = this;
        mStages = pAnim->mStages;
        mLightKeys = pAnim->mLightKeys;
        mBaseKeys = pAnim->mBaseKeys;
        mEdgeKeys = pAnim->mEdgeKeys;
        mAlphaKeys = pAnim->mAlphaKeys;
    } else {
        mKeysOwner = pAnim->mKeysOwner;
    }
    AcquireRefs();
}

void RndMatAnim::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new MatAnim");
        return;
    }
    RndAnimatable::Load(stream);
    ReleaseRefs();
    mMat = FindByName<RndMat>(stream);
    stream >> mStages;
    mKeysOwner = FindByName<RndMatAnim>(stream);
    if (sLoadRev >= kRevColorKeys) {
        stream >> mLightKeys;
        std::vector<Key<Color>> ambientKeys;
        if (sLoadRev < kRevNoAmbient) {
            stream >> ambientKeys;
        }
        stream >> mBaseKeys >> mEdgeKeys >> mAlphaKeys;
        if (sLoadRev < kRevNoAmbient) {
            if (mBaseKeys.empty()) {
                mBaseKeys = ambientKeys;
            } else if (!ambientKeys.empty()) {
                DebugPrint("%s: ignoring ambient keys\n", mName.c_str());
            }
        }
    }
    AcquireRefs();
}

void RndMatAnim::SetMat(RndMat *pMat) {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    mMat = pMat;
    if (pMat != nullptr) {
        pMat->AddRef(this);
    }
}

void RndMatAnim::ResizeStages(int nStages) {
    std::vector<Stage> &stages = mKeysOwner->mStages;
    if (static_cast<unsigned int>(nStages) < stages.size()) {
        for (auto it = stages.begin() + nStages; it != mKeysOwner->mStages.end(); ++it) {
            for (const Key<RndTex *> &key : it->mTexKeys) {
                if (key.value != nullptr) {
                    key.value->RemoveRef(this);
                }
            }
        }
    }
    mKeysOwner->mStages.resize(nStages, Stage());
    for (Stage &stage : mKeysOwner->mStages) {
        stage.mMatAnim = this;
    }
}

void RndMatAnim::ReleaseRefs() {
    if (mMat != nullptr) {
        mMat->RemoveRef(this);
    }
    if (mKeysOwner != nullptr) {
        mKeysOwner->RemoveRef(this);
    }
    for (const Stage &stage : mStages) {
        for (const Key<RndTex *> &key : stage.mTexKeys) {
            if (key.value != nullptr) {
                key.value->RemoveRef(this);
            }
        }
    }
}

void RndMatAnim::AcquireRefs() {
    if (mMat != nullptr) {
        mMat->AddRef(this);
    }
    if (mKeysOwner != nullptr) {
        mKeysOwner->AddRef(this);
    }
    for (Stage &stage : mStages) {
        stage.mMatAnim = this;
        for (const Key<RndTex *> &key : stage.mTexKeys) {
            if (key.value != nullptr) {
                key.value->AddRef(this);
            }
        }
    }
}

BinStream &operator<<(BinStream &stream, const Key<RndTex *> &key) {
    WriteName(stream, key.value);
    WriteKeyValue(stream, key.frame);
    return stream;
}

BinStream &operator>>(BinStream &stream, Key<RndTex *> &key) {
    key.value = FindByName<RndTex>(stream);
    ReadKeyValue(stream, key.frame);
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const std::vector<RndMatAnim::Stage> &stages) {
    stream << "(size:" << static_cast<unsigned int>(stages.size()) << ")";
    int nIndex = 0;
    for (const RndMatAnim::Stage &stage : stages) {
        stream << "\n" << nIndex << "\t";
        stage.Print(stream);
        ++nIndex;
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const std::vector<RndMatAnim::Stage> &stages) {
    const int nSize = static_cast<int>(stages.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const RndMatAnim::Stage &stage : stages) {
        stage.Save(stream);
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, std::vector<RndMatAnim::Stage> &stages) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    const RndMatAnim::Stage stage;
    stages.resize(nSize, stage);
    for (RndMatAnim::Stage &element : stages) {
        element.Load(stream);
    }
    return stream;
}
