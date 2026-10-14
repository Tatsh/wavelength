#include "rnd/rndparticlesysanim.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The first version with the emission rate ratio.
constexpr int kRevEmitRateRatio = 1;

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

// Move a colour range so that its low end is a new colour, retaining its width.
inline void MoveColorRange(const Color &color, Color &low, Color &high) {
    Color moved;
    moved.r = color.r + high.r;
    moved.g = color.g + high.g;
    moved.b = color.b + high.b;
    moved.a = color.a + high.a;
    moved.a = moved.a - low.a;
    moved.g = moved.g - low.g;
    moved.b = moved.b - low.b;
    moved.r = moved.r - low.r;
    low = color;
    high = moved;
}

} // namespace

const char *RndParticleSysAnim::sClassName = "ParticleSysAnim";
int RndParticleSysAnim::sRev = 1;

RndParticleSysAnim::RndParticleSysAnim(const char *pszName) : RndObject(pszName) {
    mParticleSys = nullptr;
    mFramesOwner = this;
    mEmitRateRatio = 0.0f;
}

RndParticleSysAnim::~RndParticleSysAnim() {
    ReleaseRefs();
}

float RndParticleSysAnim::EndFrame() {
    const float fStart = LastFrame(mFramesOwner->mStartColorKeys);
    const float fEnd = LastFrame(mFramesOwner->mEndColorKeys);
    const float fEmitRate = LastFrame(mFramesOwner->mEmitRateKeys);
    const float fLater = fEnd < fEmitRate ? fEmitRate : fEnd;
    return fStart < fLater ? fLater : fStart;
}

void RndParticleSysAnim::ListAnimObjects(std::list<RndObject *> &objects) {
    objects.push_back(mParticleSys);
    RndAnimatable::ListAnimObjects(objects);
}

int RndParticleSysAnim::SetFrameSelf(float fFrame) {
    if (mParticleSys == nullptr) {
        return 1;
    }
    if (!mFramesOwner->mStartColorKeys.empty()) {
        Color color;
        InterpKeys(mFramesOwner->mStartColorKeys, fFrame, color);
        MoveColorRange(color, mParticleSys->mStartColorLow, mParticleSys->mStartColorHigh);
    }
    if (!mFramesOwner->mEndColorKeys.empty()) {
        Color color;
        InterpKeys(mFramesOwner->mEndColorKeys, fFrame, color);
        MoveColorRange(color, mParticleSys->mEndColorLow, mParticleSys->mEndColorHigh);
    }
    if (!mFramesOwner->mEmitRateKeys.empty()) {
        float fEmitRate;
        InterpKeys(mFramesOwner->mEmitRateKeys, fFrame, fEmitRate);
        Vector2 &emitRate = mParticleSys->mEmitRate;
        if (emitRate.x != 0.0f) {
            mEmitRateRatio = emitRate.y / emitRate.x;
        }
        emitRate.x = fEmitRate;
        emitRate.y = fEmitRate * mEmitRateRatio;
    }
    return 1;
}

void RndParticleSysAnim::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndParticleSysAnim]\n";
    stream << "particleSys:" << static_cast<const RndObject *>(mParticleSys)
           << " framesOwner:" << static_cast<const RndObject *>(mFramesOwner)
           << " emitRateRatio:" << mEmitRateRatio << "\n";
    stream << "startColorKeys:" << mStartColorKeys << "\n";
    stream << "endColorKeys:" << mEndColorKeys << "\n";
    stream << "emitRateKeys:" << mEmitRateKeys << "\n";
}

void RndParticleSysAnim::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    WriteName(stream, mParticleSys);
    stream << mStartColorKeys << mEndColorKeys << mEmitRateKeys;
    WriteName(stream, mFramesOwner);
    stream.WriteEndian(&mEmitRateRatio, sizeof(mEmitRateRatio));
}

void RndParticleSysAnim::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    if (static_cast<RndObject *>(mParticleSys) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mParticleSys != nullptr) {
            mParticleSys = pTo != nullptr ? dynamic_cast<RndParticleSys *>(pTo) : nullptr;
        }
        if (mParticleSys != nullptr) {
            mParticleSys->AddRef(this);
        }
    }
    if (static_cast<RndObject *>(mFramesOwner) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        RndObject *pOwner = pTo != nullptr ? pTo : this;
        if (mFramesOwner != nullptr) {
            mFramesOwner = dynamic_cast<RndParticleSysAnim *>(pOwner);
        }
        if (mFramesOwner != nullptr) {
            mFramesOwner->AddRef(this);
        }
    }
}

void RndParticleSysAnim::Copy(const RndObject *pSource, int nFlags) {
    const RndParticleSysAnim *pAnim =
        pSource != nullptr ? dynamic_cast<const RndParticleSysAnim *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    ReleaseRefs();
    mParticleSys = pAnim->mParticleSys;
    mEmitRateRatio = pAnim->mEmitRateRatio;
    if ((nFlags & kCopyShareKeys) == 0 && pAnim->mFramesOwner == pAnim) {
        mFramesOwner = this;
        mStartColorKeys = pAnim->mStartColorKeys;
        mEndColorKeys = pAnim->mEndColorKeys;
        mEmitRateKeys = pAnim->mEmitRateKeys;
    } else {
        mFramesOwner = pAnim->mFramesOwner;
    }
    AcquireRefs();
}

void RndParticleSysAnim::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new ParticleSysAnim");
        return;
    }
    RndAnimatable::Load(stream);
    ReleaseRefs();
    mParticleSys = FindByName<RndParticleSys>(stream);
    stream >> mStartColorKeys >> mEndColorKeys >> mEmitRateKeys;
    mFramesOwner = FindByName<RndParticleSysAnim>(stream);
    if (nRev >= kRevEmitRateRatio) {
        stream.ReadEndian(&mEmitRateRatio, sizeof(mEmitRateRatio));
    }
    AcquireRefs();
}

void RndParticleSysAnim::ReleaseRefs() {
    if (mParticleSys != nullptr) {
        mParticleSys->RemoveRef(this);
    }
    if (mFramesOwner != nullptr) {
        mFramesOwner->RemoveRef(this);
    }
}

void RndParticleSysAnim::AcquireRefs() {
    if (mParticleSys != nullptr) {
        mParticleSys->AddRef(this);
    }
    if (mFramesOwner != nullptr) {
        mFramesOwner->AddRef(this);
    }
}
