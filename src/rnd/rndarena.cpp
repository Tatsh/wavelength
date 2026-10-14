#include "rnd/rndarena.h"

#include <cmath>

#include "os/debug.h"
#include "rnd/rndmanager.h"

namespace {

// The versions that added the animatable base, the other bases, the teleport flag, the sort flag,
// and the view animation flag.
constexpr int kRevAnimatable = 1;
constexpr int kRevBases = 2;
constexpr int kRevTeleport = 3;
constexpr int kRevSortStart = 4;
constexpr int kRevAnimateViews = 5;

// The version that added a section's window.
constexpr int kRevDelta = 1;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

int ReadFlag(BinStream &stream) {
    unsigned char nFlag;
    stream.Read(&nFlag, sizeof(nFlag));
    return nFlag != 0;
}

void WriteFlag(BinStream &stream, int nValue) {
    const unsigned char nFlag = static_cast<unsigned char>(nValue);
    stream.Write(&nFlag, sizeof(nFlag));
}

} // namespace

const char *RndArena::sClassName = "Arena";
int RndArena::sRev = 5;
int RndArena::sLoadRev;

RndArena::RndArena(const char *pszName)
    : RndObject(pszName), mLoopFrames(1000.0f), mAnimateViews(1) {
    mLoopDist.x = 0.0f;
    mLoopDist.y = 0.0f;
    mLoopDist.z = 0.0f;
    AcquireRefs();
}

RndArena::~RndArena() {
    ReleaseRefs();
}

void RndArena::Copy(const RndObject *pSource, int nFlags) {
    const RndArena *pArena = pSource != nullptr ? dynamic_cast<const RndArena *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    RndCollideable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    RndTransformable::Copy(pSource, nFlags);
    ReleaseRefs();
    mLoopDist = pArena->mLoopDist;
    mLoopFrames = pArena->mLoopFrames;
    mSections = pArena->mSections;
    mAnimateViews = pArena->mAnimateViews;
    AcquireRefs();
}

void RndArena::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    RndTransformable::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndCollideable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndArena]\n";
    stream << "loopDist:" << mLoopDist << " loopFrames:" << mLoopFrames << "\n";
    stream << "sections:" << mSections << "\n";
}

void RndArena::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    RndCollideable::Save(stream);
    RndDrawable::Save(stream);
    RndTransformable::Save(stream);
    stream.WriteEndian(&mLoopDist.x, sizeof(mLoopDist.x));
    stream.WriteEndian(&mLoopDist.y, sizeof(mLoopDist.y));
    stream.WriteEndian(&mLoopDist.z, sizeof(mLoopDist.z));
    stream.WriteEndian(&mLoopFrames, sizeof(mLoopFrames));
    stream << mSections;
    WriteFlag(stream, mAnimateViews);
}

void RndArena::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new Arena");
        return;
    }
    if (sLoadRev >= kRevAnimatable) {
        RndAnimatable::Load(stream);
    }
    if (sLoadRev >= kRevBases) {
        RndCollideable::Load(stream);
        RndDrawable::Load(stream);
        RndTransformable::Load(stream);
    }
    ReleaseRefs();
    stream.ReadEndian(&mLoopDist.x, sizeof(mLoopDist.x));
    stream.ReadEndian(&mLoopDist.y, sizeof(mLoopDist.y));
    stream.ReadEndian(&mLoopDist.z, sizeof(mLoopDist.z));
    stream.ReadEndian(&mLoopFrames, sizeof(mLoopFrames));
    stream >> mSections;
    if (sLoadRev >= kRevAnimateViews) {
        mAnimateViews = ReadFlag(stream);
    }
    AcquireRefs();
}

void RndArena::AcquireRefs() {
    for (Section &section : mSections) {
        if (section.mView != nullptr) {
            section.mView->AddRef(this);
        }
    }
    mSorts.resize(mSections.size(), Sort());
}

void RndArena::ReleaseRefs() {
    for (Section &section : mSections) {
        if (section.mView != nullptr) {
            section.mView->RemoveRef(this);
        }
    }
    mSorts.clear();
}

void RndArena::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    RndCollideable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    RndTransformable::Replace(pFrom, pTo);
    for (Section &section : mSections) {
        if (static_cast<RndObject *>(section.mView) != pFrom) {
            continue;
        }
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (section.mView == nullptr) {
            continue;
        }
        section.mView = pTo != nullptr ? dynamic_cast<RndView *>(pTo) : nullptr;
        if (section.mView != nullptr) {
            section.mView->AddRef(this);
        }
    }
    for (Sort &sort : mSorts) {
        if (static_cast<RndObject *>(sort.mView) == pFrom) {
            sort.mView = nullptr;
        }
    }
}

void RndArena::UpdateSection(Section &section) {
    if (section.mView == nullptr) {
        return;
    }
    const float fFrame = mFilteredFrame;
    const int nLoop = static_cast<int>(std::floor((fFrame - section.mFrame) / mLoopFrames));
    SetSectionLoop(section, nLoop + 1);
    const float fLocalFrame = fFrame - static_cast<float>(section.mLoop) * mLoopFrames;
    if (section.mFrame - section.mDelta <= fLocalFrame && fLocalFrame < section.mFrame) {
        if (mAnimateViews != 0) {
            section.mView->SetFrame(fLocalFrame);
        }
        section.mView->SetShowing(1);
    } else if (section.mView->mShowing != 0) {
        section.mView->SetShowing(0);
    }
}

void RndArena::SetSectionLoop(Section &section, int nLoop) {
    RndView *pView = section.mView;
    if (pView == nullptr || section.mLoop == nLoop) {
        return;
    }
    Vector3 translation = pView->mLocalXfm.mTranslation;
    if (section.mLoop < nLoop) {
        const float fLoops = static_cast<float>(nLoop - section.mLoop);
        const float fX = mLoopDist.x * fLoops;
        const float fY = mLoopDist.y * fLoops;
        const float fZ = mLoopDist.z * fLoops;
        translation.x = pView->mLocalXfm.mTranslation.x + fX;
        translation.y = pView->mLocalXfm.mTranslation.y + fY;
        translation.z = pView->mLocalXfm.mTranslation.z + fZ;
    } else if (nLoop < section.mLoop) {
        const float fLoops = static_cast<float>(section.mLoop - nLoop);
        const float fX = mLoopDist.x * fLoops;
        const float fY = mLoopDist.y * fLoops;
        const float fZ = mLoopDist.z * fLoops;
        translation.x = pView->mLocalXfm.mTranslation.x - fX;
        translation.y = pView->mLocalXfm.mTranslation.y - fY;
        translation.z = pView->mLocalXfm.mTranslation.z - fZ;
    }
    section.mLoop = nLoop;
    if (section.mTeleport != 0) {
        pView->mDirty = 1;
        pView->mLocalXfm.mTranslation = translation;
    }
}

int RndArena::SetFrameSelf([[maybe_unused]] float fFrame) {
    auto sort = mSorts.begin();
    for (Section &section : mSections) {
        UpdateSection(section);
        if (section.mView != nullptr && section.mView->mShowing != 0) {
            sort->mView = section.mView;
            sort->mSortKey = section.mFrame + static_cast<float>(section.mLoop) * mLoopFrames;
            if (section.mSortStart != 0) {
                sort->mSortKey -= section.mDelta;
            }
        } else {
            sort->mView = nullptr;
            sort->mSortKey = 0.0f;
        }
        ++sort;
    }
    mSorts.sort();
    return 1;
}

int RndArena::UpdateWorldXfm(RndTransformable *pParent, int bForce) {
    const int nUpdated = RndTransformable::UpdateWorldXfm(pParent, bForce);
    for (Section &section : mSections) {
        if (section.mView != nullptr && section.mView->mShowing != 0) {
            section.mView->UpdateWorldXfm(this, nUpdated);
        }
    }
    return nUpdated;
}

int RndArena::DrawShowing() {
    for (Sort &sort : mSorts) {
        if (sort.mView != nullptr) {
            sort.mView->Draw();
        }
    }
    return 1;
}

void RndArena::Collide(const Segment &segment, std::list<Collision> &collisions) {
    for (Section &section : mSections) {
        if (section.mView != nullptr && section.mView->mShowing != 0) {
            section.mView->Collide(segment, collisions);
        }
    }
}

PrnStream &operator<<(PrnStream &stream, const RndArena::Section &section) {
    stream << "\n\tview:" << static_cast<const RndObject *>(section.mView)
           << " frame:" << section.mFrame << " loop:" << section.mLoop << "\n";
    stream << "\n\tdelta:" << section.mDelta << " teleport:" << (section.mTeleport != 0)
           << " sortStart:" << (section.mSortStart != 0);
    return stream;
}

BinStream &operator<<(BinStream &stream, const RndArena::Section &section) {
    const RndObject *pView = section.mView;
    stream.WriteString(pView != nullptr ? pView->mName.c_str() : "");
    stream.WriteEndian(&section.mFrame, sizeof(section.mFrame));
    stream.WriteEndian(&section.mLoop, sizeof(section.mLoop));
    stream.WriteEndian(&section.mDelta, sizeof(section.mDelta));
    WriteFlag(stream, section.mTeleport);
    WriteFlag(stream, section.mSortStart);
    return stream;
}

BinStream &operator>>(BinStream &stream, RndArena::Section &section) {
    section.mView = FindByName<RndView>(stream);
    stream.ReadEndian(&section.mFrame, sizeof(section.mFrame));
    stream.ReadEndian(&section.mLoop, sizeof(section.mLoop));
    if (RndArena::sLoadRev >= kRevDelta) {
        stream.ReadEndian(&section.mDelta, sizeof(section.mDelta));
    }
    if (RndArena::sLoadRev >= kRevTeleport) {
        section.mTeleport = ReadFlag(stream);
    }
    if (RndArena::sLoadRev >= kRevSortStart) {
        section.mSortStart = ReadFlag(stream);
    }
    return stream;
}
