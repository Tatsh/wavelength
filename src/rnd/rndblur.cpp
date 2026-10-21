#include "rnd/rndblur.h"

#include "math/vector3.h"
#include "os/debug.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"
#include "rnd/rndtransformable.h"

namespace {

// The first version with the falloff, and the first with the text subject.
constexpr int kRevFalloff = 1;
constexpr int kRevText = 2;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.GetLength() == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

inline bool SameRow(const Vector3 &a, const Vector3 &b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline bool SameXfm(const Transform &a, const Transform &b) {
    return SameRow(a.mBasisX, b.mBasisX) && SameRow(a.mBasisY, b.mBasisY) &&
           SameRow(a.mBasisZ, b.mBasisZ) && SameRow(a.mTranslation, b.mTranslation);
}

} // namespace

const char *RndBlur::sClassName = "Blur";
int RndBlur::sRev = 2;
int RndBlur::sLoadRev;

RndBlur::RndBlur(const char *pszName)
    : RndObject(pszName), mMesh(nullptr), mText(nullptr), mLength(0), mRate(1), mFalloff(1.0f),
      mCountdown(mRate) {
    AcquireRefs();
}

RndBlur::~RndBlur() {
    ReleaseRefs();
}

void RndBlur::Copy(const RndObject *pSource, int nFlags) {
    const RndBlur *pBlur = pSource != nullptr ? dynamic_cast<const RndBlur *>(pSource) : nullptr;
    RndDrawable::Copy(pSource, nFlags);
    ReleaseRefs();
    mMesh = pBlur->mMesh;
    mText = pBlur->mText;
    mLength = pBlur->mLength;
    mRate = pBlur->mRate;
    mFalloff = pBlur->mFalloff;
    AcquireRefs();
}

void RndBlur::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndDrawable::DumpText(stream);
    if (stream.GetDumpLevel() <= 0) {
        return;
    }
    stream << "[RndBlur]\n";
    stream << "mesh:" << static_cast<const RndObject *>(mMesh) << " length:" << mLength
           << " rate:" << mRate << "\n";
    stream << "falloff:" << mFalloff << " text:" << static_cast<const RndObject *>(mText) << "\n";
}

void RndBlur::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndDrawable::Save(stream);
    const RndObject *pMesh = mMesh;
    stream.WriteString(pMesh != nullptr ? pMesh->GetName() : "");
    stream.WriteEndian(&mLength, sizeof(mLength));
    stream.WriteEndian(&mRate, sizeof(mRate));
    stream.WriteEndian(&mFalloff, sizeof(mFalloff));
    const RndObject *pText = mText;
    stream.WriteString(pText != nullptr ? pText->GetName() : "");
}

void RndBlur::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new Blur");
        return;
    }
    RndDrawable::Load(stream);
    ReleaseRefs();
    mMesh = FindByName<RndMesh>(stream);
    stream.ReadEndian(&mLength, sizeof(mLength));
    stream.ReadEndian(&mRate, sizeof(mRate));
    if (sLoadRev >= kRevFalloff) {
        stream.ReadEndian(&mFalloff, sizeof(mFalloff));
    }
    if (sLoadRev >= kRevText) {
        mText = FindByName<RndText>(stream);
    }
    AcquireRefs();
}

void RndBlur::AcquireRefs() {
    if (mMesh != nullptr) {
        mMesh->AddRef(this);
    }
    if (mText != nullptr) {
        mText->AddRef(this);
    }
    mXfms.clear();
}

void RndBlur::ReleaseRefs() {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
    if (mText != nullptr) {
        mText->RemoveRef(this);
    }
}

void RndBlur::Replace(RndObject *pFrom, RndObject *pTo) {
    RndDrawable::Replace(pFrom, pTo);
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
    if (static_cast<RndObject *>(mText) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mText != nullptr) {
            mText = pTo != nullptr ? dynamic_cast<RndText *>(pTo) : nullptr;
        }
        if (mText != nullptr) {
            mText->AddRef(this);
        }
    }
}

void RndBlur::ListDrawObjects(std::list<RndObject *> &objects) {
    if (mMesh != nullptr) {
        mMesh->ListDrawObjects(objects);
    }
    if (mText != nullptr) {
        mText->ListDrawObjects(objects);
    }
}

void RndBlur::ListDrawables(std::list<RndDrawable *> &drawables) {
    if (mMesh != nullptr || mText != nullptr) {
        std::list<RndDrawable *> subjectDrawables;
        RndTransformable *pTrans;
        if (mText != nullptr) {
            pTrans = mText;
        } else {
            pTrans = mMesh;
        }
        RndDrawable *pSubject;
        if (mText != nullptr) {
            pSubject = mText;
        } else {
            pSubject = mMesh;
        }
        const Transform savedLocal = pTrans->GetLocalXfm();
        const Transform savedWorld = pTrans->GetWorldXfm();
        pSubject->ListDrawables(subjectDrawables);
        for (const Transform &xfm : mXfms) {
            pTrans->GetLocalXfm() = xfm;
            pTrans->SetDirty(1);
            pTrans->UpdateWorldXfm(nullptr, 0);
            pSubject->ListDrawables(subjectDrawables);
            if (!subjectDrawables.empty()) {
                drawables.push_back(this);
                break;
            }
        }
        // Yes, the binary rebuilds the world transform from the saved world transform as a local
        // one before it puts the saved local transform back.
        pTrans->GetLocalXfm() = savedWorld;
        pTrans->SetDirty(1);
        pTrans->UpdateWorldXfm(nullptr, 0);
        pTrans->GetLocalXfm() = savedLocal;
        pTrans->SetDirty(1);
    }
    RndDrawable::ListDrawables(drawables);
}

int RndBlur::DrawShowing() {
    if (mMesh == nullptr && mText == nullptr) {
        return 1;
    }
    if (mText != nullptr) {
        mText->Draw();
    } else {
        mMesh->Draw();
    }
    if (mLength == 0) {
        return 1;
    }

    RndMat *pMat = nullptr;
    if (mText != nullptr) {
        RndFont *pFont = mText->GetFont();
        if (pFont == nullptr) {
            return 1;
        }
        pMat = pFont->GetMat();
        if (pMat == nullptr) {
            return 1;
        }
    }
    if (mMesh != nullptr) {
        pMat = mMesh->GetMat();
        if (pMat == nullptr) {
            return 1;
        }
    }

    --mCountdown;
    RndTransformable *pTrans;
    if (mText != nullptr) {
        pTrans = mText;
    } else {
        pTrans = mMesh;
    }
    RndDrawable *pSubject;
    if (mText != nullptr) {
        pSubject = mText;
    } else {
        pSubject = mMesh;
    }
    const Transform savedLocal = pTrans->GetLocalXfm();
    const Transform savedWorld = pTrans->GetWorldXfm();

    const float fAlpha = pMat->GetBaseColor().a;
    const float fStart = fAlpha * mFalloff;
    float fStep = fStart / static_cast<float>(mLength * mRate);
    float fStepAlpha = fStart - fStep * static_cast<float>(mRate - mCountdown);
    fStep = fStep * static_cast<float>(mRate);

    // The zeros are never stored. The mesh depth state is read and restored only without a text.
    const int nZMode = mText == nullptr ? mMesh->GetZMode() : 0;
    const int nZFunc = mText == nullptr ? mMesh->GetZFunc() : 0;
    if (mText == nullptr && nZMode != RndMesh::kZModeDisable) {
        mMesh->SetZMode(RndMesh::kZModeZReadOnly);
    }

    const Transform *pPrevious = &savedWorld;
    for (const Transform &xfm : mXfms) {
        if (!SameXfm(xfm, *pPrevious)) {
            pMat->SetAlpha(fStepAlpha);
            pTrans->GetLocalXfm() = xfm;
            pTrans->SetDirty(1);
            pTrans->UpdateWorldXfm(nullptr, 0);
            pSubject->Draw();
        }
        fStepAlpha -= fStep;
        pPrevious = &xfm;
    }

    // Yes, the binary rebuilds the world transform from the saved world transform as a local one
    // before it puts the saved local transform back.
    pTrans->GetLocalXfm() = savedWorld;
    pTrans->SetDirty(1);
    pTrans->UpdateWorldXfm(nullptr, 0);
    pTrans->GetLocalXfm() = savedLocal;
    pTrans->SetDirty(1);
    pMat->SetAlpha(fAlpha);
    if (mText == nullptr) {
        mMesh->SetZFunc(nZFunc);
        mMesh->SetZMode(nZMode);
    }

    if (mCountdown == 0) {
        if (static_cast<int>(mXfms.size()) == mLength) {
            mXfms.pop_back();
        }
        mXfms.push_front(savedWorld);
        mCountdown = mRate;
    }
    return 1;
}

void RndBlur::SetLength(int nLength) {
    mLength = nLength < 0 ? 0 : nLength;
    mXfms.clear();
}

void RndBlur::SetRate(int nRate) {
    mRate = nRate < 1 ? 1 : nRate;
    mXfms.clear();
}
