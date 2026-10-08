#include "rnd/rndview.h"

#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"

namespace {

// The first version without the legacy camera, and the first with the children owner.
constexpr int kRevNoCamera = 3;
constexpr int kRevChildrenOwner = 4;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

} // namespace

const char *RndView::sClassName = "View";
int RndView::sRev = 4;

RndView::RndView(const char *pszName) : RndObject(pszName) {
    mChildrenOwner = this;
    mVisibleRange.x = 0.0f;
    mVisibleRange.y = 0.0f;
    mInRange = 1;
}

RndView::~RndView() {
    ReleaseRefs();
}

inline void RndView::SyncChildren() {
    for (RndAnimatable *pAnim : mChildrenOwner->mAnims) {
        pAnim->SetFrame(mFilteredFrame);
    }
    for (RndTransformable *pTrans : mChildrenOwner->mTransList) {
        pTrans->UpdateWorldXfm(this, 1);
        pTrans->mDirty = 1;
    }
}

int RndView::SetFrameSelf([[maybe_unused]] float fFrame) {
    if (mVisibleRange.x < mVisibleRange.y) {
        mInRange = mVisibleRange.x <= mFrame && mFrame < mVisibleRange.y;
    }
    return mInRange;
}

void RndView::ListDrawables(std::list<RndDrawable *> &drawables) {
    if (mChildrenOwner != this) {
        SyncChildren();
        std::list<RndDrawable *> children;
        static_cast<RndDrawable *>(mChildrenOwner)->ListDrawables(children);
        if (!children.empty()) {
            drawables.push_back(this);
        }
    }
    RndDrawable::ListDrawables(drawables);
}

int RndView::DrawShowing() {
    if (mInRange == 0) {
        return 0;
    }
    if (mChildrenOwner == this) {
        return 1;
    }
    SyncChildren();
    for (RndDrawable *pDraw : mChildrenOwner->mDraws) {
        pDraw->Draw();
    }
    return 1;
}

int RndView::UpdateWorldXfm(RndTransformable *pParent, int bForce) {
    if (mInRange == 0) {
        return 0;
    }
    return RndTransformable::UpdateWorldXfm(pParent, bForce);
}

void RndView::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    RndTransformable::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndCollideable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndView]\n";
    stream << "childrenOwner:" << static_cast<const RndObject *>(mChildrenOwner)
           << " visibleRange:" << mVisibleRange << "\n";
}

void RndView::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    RndTransformable::Save(stream);
    RndDrawable::Save(stream);
    RndCollideable::Save(stream);
    const RndObject *pOwner = mChildrenOwner;
    stream.WriteString(pOwner != nullptr ? pOwner->mName.c_str() : "");
    stream.WriteEndian(&mVisibleRange.x, sizeof(mVisibleRange.x));
    stream.WriteEndian(&mVisibleRange.y, sizeof(mVisibleRange.y));
}

void RndView::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    RndTransformable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
    RndCollideable::Replace(pFrom, pTo);
    if (static_cast<RndObject *>(mChildrenOwner) != pFrom) {
        return;
    }
    if (pFrom != nullptr) {
        pFrom->RemoveRef(this);
    }
    RndObject *pOwner = pTo != nullptr ? pTo : this;
    if (mChildrenOwner != nullptr) {
        mChildrenOwner = dynamic_cast<RndView *>(pOwner);
    }
    if (mChildrenOwner != nullptr) {
        mChildrenOwner->AddRef(this);
    }
}

void RndView::Copy(const RndObject *pSource, int nFlags) {
    const RndView *pView = pSource != nullptr ? dynamic_cast<const RndView *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    RndTransformable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    RndCollideable::Copy(pSource, nFlags);
    ReleaseRefs();
    mVisibleRange = pView->mVisibleRange;
    if ((nFlags & kCopyShareChildren) == 0 && pView->mChildrenOwner == pView) {
        mChildrenOwner = this;
    } else {
        mChildrenOwner = pView->mChildrenOwner;
    }
    AcquireRefs();
}

void RndView::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new View");
        return;
    }
    RndAnimatable::Load(stream);
    RndTransformable::Load(stream);
    RndDrawable::Load(stream);
    RndCollideable::Load(stream);
    ReleaseRefs();
    if (nRev < kRevNoCamera) {
        FindByName<RndCam>(stream); // Yes, the binary discards the legacy camera.
    }
    if (nRev >= kRevChildrenOwner) {
        mChildrenOwner = FindByName<RndView>(stream);
        stream.ReadEndian(&mVisibleRange.x, sizeof(mVisibleRange.x));
        stream.ReadEndian(&mVisibleRange.y, sizeof(mVisibleRange.y));
    }
    AcquireRefs();
}

void RndView::SetVisibleRange(float fStart, float fEnd) {
    mVisibleRange.x = fStart;
    mInRange = 1;
    mVisibleRange.y = fEnd;
}

void RndView::SetInRange(int nInRange) {
    mInRange = nInRange;
}

void RndView::ReleaseRefs() {
    if (mChildrenOwner != nullptr) {
        mChildrenOwner->RemoveRef(this);
    }
}

void RndView::AcquireRefs() {
    if (mChildrenOwner != nullptr) {
        mChildrenOwner->AddRef(this);
    }
    mInRange = 1;
}
