#include "rnd/rnddrawable.h"

#include <algorithm>

#include "os/debug.h"
#include "rnd/rndmanager.h"

int RndDrawable::sRev = 0;

RndDrawable::~RndDrawable() {
    ReleaseDrawRefs();
}

void RndDrawable::ListDrawObjects([[maybe_unused]] std::list<RndObject *> &objects) {
}

void RndDrawable::ListDrawables(std::list<RndDrawable *> &drawables) {
    for (RndDrawable *pDraw : mDraws) {
        pDraw->ListDrawables(drawables);
    }
}

void RndDrawable::SetHighlight(int nHighlight) {
    mHighlight = nHighlight;
}

void RndDrawable::DumpText(PrnStream &stream) {
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndDrawable]\n";
    stream << "show:" << (mShowing != 0) << " highlight:" << (mHighlight != 0)
           << " draws:" << mDraws << "\n";
}

void RndDrawable::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    const char bShowing = static_cast<char>(mShowing);
    stream.Write(&bShowing, sizeof(bShowing));
    stream << mDraws;
}

void RndDrawable::Replace(RndObject *pFrom, RndObject *pTo) {
    for (auto it = mDraws.begin(); it != mDraws.end();) {
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
                *it = pTo != nullptr ? dynamic_cast<RndDrawable *>(pTo) : nullptr;
            }
            if (*it != nullptr) {
                (*it)->AddRef(this);
            }
        }
        if (*it == nullptr) {
            it = mDraws.erase(it);
        } else {
            ++it;
        }
    }
}

void RndDrawable::Copy(const RndObject *pSource, int nFlags) {
    const RndDrawable *pDrawable =
        pSource != nullptr ? dynamic_cast<const RndDrawable *>(pSource) : nullptr;
    ReleaseDrawRefs();
    mShowing = pDrawable->mShowing;
    if ((nFlags & kCopyChildLists) != 0) {
        mDraws = pDrawable->mDraws;
    }
    AcquireDrawRefs();
}

void RndDrawable::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugWarn("Can't load new Drawable");
    }
    ReleaseDrawRefs();
    char bShowing;
    stream.Read(&bShowing, sizeof(bShowing));
    mShowing = bShowing != 0;
    stream >> mDraws;
    AcquireDrawRefs();
}

RndDrawable *RndDrawable::Parent() {
    for (RndObject *pRef : mRefs) {
        RndDrawable *pDrawable = pRef != nullptr ? dynamic_cast<RndDrawable *>(pRef) : nullptr;
        if (pDrawable != nullptr &&
            std::find(pDrawable->mDraws.begin(), pDrawable->mDraws.end(), this) !=
                pDrawable->mDraws.end()) {
            return pDrawable;
        }
    }
    return nullptr;
}

void RndDrawable::Draw() {
    if (mShowing == 0 || DrawShowing() == 0) {
        return;
    }
    for (RndDrawable *pDraw : mDraws) {
        pDraw->Draw();
    }
}

bool RndDrawable::AddDraw(RndDrawable *pDraw, RndDrawable *pBefore) {
    if (std::find(mDraws.begin(), mDraws.end(), pDraw) != mDraws.end()) {
        DebugNotify("%s already in %s", pDraw->mName.c_str(), mName.c_str());
        return false;
    }
    const auto position = std::find(mDraws.begin(), mDraws.end(), pBefore);
    if (pDraw != nullptr) {
        pDraw->AddRef(this);
    }
    mDraws.insert(position, pDraw);
    return true;
}

void RndDrawable::RemoveDraw(RndDrawable *pDraw) {
    if (std::find(mDraws.begin(), mDraws.end(), pDraw) == mDraws.end()) {
        return;
    }
    if (pDraw != nullptr) {
        pDraw->RemoveRef(this);
    }
    mDraws.remove(pDraw);
}

void RndDrawable::ClearDraws() {
    for (auto it = mDraws.begin(); it != mDraws.end();) {
        if (*it != nullptr) {
            (*it)->RemoveRef(this);
        }
        it = mDraws.erase(it);
    }
}

void RndDrawable::ReleaseDrawRefs() {
    for (RndDrawable *pDraw : mDraws) {
        if (pDraw != nullptr) {
            pDraw->RemoveRef(this);
        }
    }
}

void RndDrawable::AcquireDrawRefs() {
    for (RndDrawable *pDraw : mDraws) {
        if (pDraw != nullptr) {
            pDraw->AddRef(this);
        }
    }
}
