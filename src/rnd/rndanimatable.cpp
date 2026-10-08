#include "rnd/rndanimatable.h"

#include <algorithm>
#include <cmath>

#include "os/debug.h"
#include "rnd/rndmanager.h"

int RndAnimatable::sRev = 0;

void RndAnimatable::ScaleOffset::Print(PrnStream &stream) const {
    stream << "(scale:" << mScale << " offset:" << mOffset << ")";
}

float RndAnimatable::MinMaxLoop::Apply(float fValue) {
    if (mLoop != 0) {
        const float fSpan = mMax - mMin;
        float fWrapped = std::fmod(fValue - mMin, fSpan);
        if (fWrapped < 0.0f) {
            fWrapped += fSpan;
        }
        return mMin + fWrapped;
    }
    return std::max(std::min(fValue, mMax), mMin);
}

float RndAnimatable::MinMaxLoop::Unapply(float fValue) {
    return fValue;
}

void RndAnimatable::MinMaxLoop::Print(PrnStream &stream) const {
    stream << "(min:" << mMin << " max:" << mMax << " loop:" << (mLoop != 0) << ")";
}

float RndAnimatable::ZeroOrder::Apply(float fValue) {
    const float fDelta = fValue - mLevel;
    if (mMaxDelta < fDelta) {
        mLevel += mMaxDelta;
        return mLevel;
    }
    if (fDelta < -mMaxDelta) {
        mLevel -= mMaxDelta;
        return mLevel;
    }
    mLevel = fValue;
    return fValue;
}

float RndAnimatable::ZeroOrder::Unapply(float fValue) {
    if (fValue < mLevel) {
        return mLevel + mMaxDelta;
    }
    if (mLevel < fValue) {
        return mLevel - mMaxDelta;
    }
    return mLevel;
}

void RndAnimatable::ZeroOrder::Print(PrnStream &stream) const {
    stream << "(level:" << mLevel << " maxDelta:" << mMaxDelta << ")";
}

void RndAnimatable::FirstOrder::Print(PrnStream &stream) const {
    stream << "(level:" << mLevel << " ratio:" << mRatio << ")";
}

float RndAnimatable::SecondOrder::Apply(float fValue) {
    mVel += (mSpring * (fValue - mLevel)) - (mDamper * mVel);
    mLevel += mVel;
    return mLevel;
}

float RndAnimatable::SecondOrder::Unapply([[maybe_unused]] float fValue) {
    return mLevel - mVel;
}

void RndAnimatable::SecondOrder::Print(PrnStream &stream) const {
    stream << "(level:" << mLevel << " spring:" << mSpring << " damper:" << ")";
    // Yes, the binary closes the bracket before the damper and the velocity.
    stream << mDamper << " vel" << mVel;
}

RndAnimatable::~RndAnimatable() {
    ReleaseObjects();
}

float RndAnimatable::EndFrame() {
    float fEnd = 0.0f;
    for (RndAnimatable *pAnim : mAnims) {
        const float fChildEnd = pAnim->UnfilterFrame(pAnim->EndFrame());
        if (fEnd < fChildEnd) {
            fEnd = fChildEnd;
        }
    }
    return fEnd;
}

void RndAnimatable::ListAnimObjects(std::list<RndObject *> &objects) {
    for (RndAnimatable *pAnim : mAnims) {
        pAnim->ListAnimObjects(objects);
    }
}

void RndAnimatable::DumpText(PrnStream &stream) {
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndAnimatable]\n";
    stream << "filters:" << mFilters << "\n";
    stream << "anims:" << mAnims << "\n";
}

void RndAnimatable::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    stream << mFilters << mAnims;
}

void RndAnimatable::Replace(RndObject *pFrom, RndObject *pTo) {
    for (auto it = mAnims.begin(); it != mAnims.end();) {
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
                *it = pTo != nullptr ? dynamic_cast<RndAnimatable *>(pTo) : nullptr;
            }
            if (*it != nullptr) {
                (*it)->AddRef(this);
            }
        }
        if (*it == nullptr) {
            it = mAnims.erase(it);
        } else {
            ++it;
        }
    }
}

void RndAnimatable::Copy(const RndObject *pSource, int nFlags) {
    const RndAnimatable *pAnimatable =
        pSource != nullptr ? dynamic_cast<const RndAnimatable *>(pSource) : nullptr;
    ReleaseObjects();
    for (Filter *pFilter : pAnimatable->mFilters) {
        Filter *pCopy = NewFilter(pFilter->Type());
        pCopy->Copy(pFilter);
        mFilters.push_back(pCopy);
    }
    if ((nFlags & kCopyChildLists) != 0) {
        mAnims = pAnimatable->mAnims;
    }
    AcquireAnimRefs();
}

void RndAnimatable::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugWarn("Can't load new Animatable");
    }
    ReleaseObjects();
    stream >> mFilters >> mAnims;
    AcquireAnimRefs();
}

RndAnimatable *RndAnimatable::Parent() {
    for (RndObject *pRef : mRefs) {
        RndAnimatable *pAnimatable =
            pRef != nullptr ? dynamic_cast<RndAnimatable *>(pRef) : nullptr;
        if (pAnimatable != nullptr &&
            std::find(pAnimatable->mAnims.begin(), pAnimatable->mAnims.end(), this) !=
                pAnimatable->mAnims.end()) {
            return pAnimatable;
        }
    }
    return nullptr;
}

void RndAnimatable::SetFrame(float fFrame) {
    mFrame = fFrame;
    mFilteredFrame = FilterFrame(fFrame);
    if (SetFrameSelf(mFilteredFrame) == 0) {
        return;
    }
    for (RndAnimatable *pAnim : mAnims) {
        pAnim->SetFrame(mFilteredFrame);
    }
}

float RndAnimatable::FilterFrame(float fValue) {
    for (Filter *pFilter : mFilters) {
        fValue = pFilter->Apply(fValue);
    }
    return fValue;
}

float RndAnimatable::UnfilterFrame(float fValue) {
    for (auto it = mFilters.rbegin(); it != mFilters.rend(); ++it) {
        fValue = (*it)->Unapply(fValue);
    }
    return fValue;
}

bool RndAnimatable::AddAnim(RndAnimatable *pAnim) {
    if (std::find(mAnims.begin(), mAnims.end(), pAnim) != mAnims.end()) {
        DebugNotify("%s already in %s", pAnim->mName.c_str(), mName.c_str());
        return false;
    }
    if (pAnim != nullptr) {
        pAnim->AddRef(this);
    }
    mAnims.push_back(pAnim);
    return true;
}

void RndAnimatable::RemoveAnim(RndAnimatable *pAnim) {
    if (std::find(mAnims.begin(), mAnims.end(), pAnim) == mAnims.end()) {
        return;
    }
    if (pAnim != nullptr) {
        pAnim->RemoveRef(this);
    }
    mAnims.remove(pAnim);
}

void RndAnimatable::RemoveAllAnims() {
    for (RndAnimatable *pAnim : mAnims) {
        if (pAnim != nullptr) {
            pAnim->RemoveRef(this);
        }
    }
    mAnims.clear();
}

RndAnimatable::Filter *RndAnimatable::NewFilter(int nType) {
    switch (nType) {
    case kFilterScaleOffset:
        return new ScaleOffset;
    case kFilterMinMaxLoop:
        return new MinMaxLoop;
    case kFilterZeroOrder:
        return new ZeroOrder;
    case kFilterFirstOrder:
        return new FirstOrder;
    case kFilterSecondOrder:
        return new SecondOrder;
    default:
        DebugWarn("Couldn't create anim filter type %d", nType);
        return nullptr;
    }
}

void RndAnimatable::ReleaseObjects() {
    for (RndAnimatable *pAnim : mAnims) {
        if (pAnim != nullptr) {
            pAnim->RemoveRef(this);
        }
    }
    for (Filter *pFilter : mFilters) {
        // Yes, the binary frees a stage without running a destructor. Filter declares none.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
        delete pFilter;
#pragma GCC diagnostic pop
    }
    mFilters.clear();
}

void RndAnimatable::AcquireAnimRefs() {
    for (RndAnimatable *pAnim : mAnims) {
        if (pAnim != nullptr) {
            pAnim->AddRef(this);
        }
    }
}

PrnStream &operator<<(PrnStream &stream, RndAnimatable::Filter *pFilter) {
    stream << "(type:" << pFilter->Type() << " params:";
    pFilter->Print(stream);
    stream << ")";
    return stream;
}

PrnStream &operator<<(PrnStream &stream, RndAnimatable::FilterType eType) {
    switch (eType) {
    case RndAnimatable::kFilterScaleOffset:
        return stream << "ScaleOffset";
    case RndAnimatable::kFilterMinMaxLoop:
        return stream << "MinMaxLoop";
    case RndAnimatable::kFilterZeroOrder:
        return stream << "ZeroOrder";
    case RndAnimatable::kFilterFirstOrder:
        return stream << "FirstOrder";
    case RndAnimatable::kFilterSecondOrder:
        return stream << "SecondOrder";
    default:
        return stream;
    }
}

BinStream &operator>>(BinStream &stream, RndAnimatable::Filter *&pFilter) {
    int nType;
    stream.ReadEndian(&nType, sizeof(nType));
    pFilter = RndAnimatable::NewFilter(nType);
    pFilter->Load(stream);
    return stream;
}

BinStream &operator<<(BinStream &stream, const std::list<RndAnimatable::Filter *> &filters) {
    const int nSize = static_cast<int>(filters.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (RndAnimatable::Filter *pFilter : filters) {
        const int nType = pFilter->Type();
        stream.WriteEndian(&nType, sizeof(nType));
        pFilter->Save(stream);
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, std::list<RndAnimatable::Filter *> &filters) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    filters.resize(nSize, nullptr);
    for (RndAnimatable::Filter *&pFilter : filters) {
        stream >> pFilter;
    }
    return stream;
}
