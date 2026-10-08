#include "rnd/rndcollideable.h"

#include <algorithm>

#include "os/debug.h"
#include "rnd/rndmanager.h"

int RndCollideable::sRev = 0;

RndCollideable::~RndCollideable() {
    ReleaseCollideRefs();
}

void RndCollideable::Collide(const Segment &segment, std::list<Collision> &collisions) {
    for (RndCollideable *pCollide : mCollides) {
        pCollide->Collide(segment, collisions);
    }
}

void RndCollideable::Collide(const Vector2 &point, std::list<Collision> &collisions) {
    for (RndCollideable *pCollide : mCollides) {
        pCollide->Collide(point, collisions);
    }
}

void RndCollideable::DumpText(PrnStream &stream) {
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndCollideable]\n";
    stream << "collides:" << mCollides << "\n";
}

void RndCollideable::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    stream << mCollides;
}

void RndCollideable::Replace(RndObject *pFrom, RndObject *pTo) {
    for (auto it = mCollides.begin(); it != mCollides.end();) {
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
                *it = pTo != nullptr ? dynamic_cast<RndCollideable *>(pTo) : nullptr;
            }
            if (*it != nullptr) {
                (*it)->AddRef(this);
            }
        }
        if (*it == nullptr) {
            it = mCollides.erase(it);
        } else {
            ++it;
        }
    }
}

void RndCollideable::Copy(const RndObject *pSource, int nFlags) {
    const RndCollideable *pCollideable =
        pSource != nullptr ? dynamic_cast<const RndCollideable *>(pSource) : nullptr;
    ReleaseCollideRefs();
    if ((nFlags & kCopyChildLists) != 0) {
        mCollides = pCollideable->mCollides;
    }
    AcquireCollideRefs();
}

void RndCollideable::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugWarn("Can't load new Collideable");
    }
    ReleaseCollideRefs();
    stream >> mCollides;
    AcquireCollideRefs();
}

RndCollideable *RndCollideable::Parent() {
    for (RndObject *pRef : mRefs) {
        RndCollideable *pCollideable =
            pRef != nullptr ? dynamic_cast<RndCollideable *>(pRef) : nullptr;
        if (pCollideable != nullptr &&
            std::find(pCollideable->mCollides.begin(), pCollideable->mCollides.end(), this) !=
                pCollideable->mCollides.end()) {
            return pCollideable;
        }
    }
    return nullptr;
}

bool RndCollideable::AddCollide(RndCollideable *pCollide) {
    if (std::find(mCollides.begin(), mCollides.end(), pCollide) != mCollides.end()) {
        DebugNotify("%s already in %s", pCollide->mName.c_str(), mName.c_str());
        return false;
    }
    if (pCollide != nullptr) {
        pCollide->AddRef(this);
    }
    mCollides.push_back(pCollide);
    return true;
}

void RndCollideable::RemoveCollide(RndCollideable *pCollide) {
    if (std::find(mCollides.begin(), mCollides.end(), pCollide) == mCollides.end()) {
        return;
    }
    if (pCollide != nullptr) {
        pCollide->RemoveRef(this);
    }
    mCollides.remove(pCollide);
}

void RndCollideable::ReleaseCollideRefs() {
    for (RndCollideable *pCollide : mCollides) {
        if (pCollide != nullptr) {
            pCollide->RemoveRef(this);
        }
    }
}

void RndCollideable::AcquireCollideRefs() {
    for (RndCollideable *pCollide : mCollides) {
        if (pCollide != nullptr) {
            pCollide->AddRef(this);
        }
    }
}
