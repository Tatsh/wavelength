#include "rnd/rndbutton.h"

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

// Repoint the entries of a palette equal to the object being replaced.
template <typename T>
void ReplaceEntries(RndObject *pOwner,
                    std::vector<T *> &entries,
                    RndObject *pFrom,
                    RndObject *pTo) {
    for (T *&pEntry : entries) {
        if (pEntry != pFrom) {
            continue;
        }
        if (pFrom != nullptr) {
            pFrom->RemoveRef(pOwner);
        }
        if (pEntry == nullptr) {
            continue;
        }
        pEntry = pTo != nullptr ? dynamic_cast<T *>(pTo) : nullptr;
        if (pEntry != nullptr) {
            pEntry->AddRef(pOwner);
        }
    }
}

} // namespace

const char *RndButton::sClassName = "Button";
int RndButton::sRev = 0;
int RndButton::sLoadRev;

RndButton::RndButton(const char *pszName)
    : RndObject(pszName), mState(0), mMesh(nullptr), mText(nullptr), mMats(kStateCount, nullptr),
      mFonts(kStateCount, nullptr) {
    AcquireRefs();
}

RndButton::~RndButton() {
    ReleaseRefs();
}

void RndButton::Copy(const RndObject *pSource, [[maybe_unused]] int nFlags) {
    const RndButton *pButton =
        pSource != nullptr ? dynamic_cast<const RndButton *>(pSource) : nullptr;
    ReleaseRefs();
    mState = pButton->mState;
    mMesh = nullptr; // Yes, the binary clears the targets rather than copying them.
    mText = nullptr;
    mMats = pButton->mMats;
    mFonts = pButton->mFonts;
    AcquireRefs();
}

void RndButton::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndButton]\n";
    stream << "state: " << mState << " mesh: " << static_cast<const RndObject *>(mMesh)
           << " text: " << static_cast<const RndObject *>(mText) << "\n";
    stream << "mats: " << mMats << "\n";
    stream << "fonts: " << mFonts << "\n";
}

void RndButton::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    stream.WriteEndian(&mState, sizeof(mState));
    const RndObject *pMesh = mMesh;
    stream.WriteString(pMesh != nullptr ? pMesh->mName.c_str() : "");
    const RndObject *pText = mText;
    stream.WriteString(pText != nullptr ? pText->mName.c_str() : "");
    stream << mMats << mFonts;
}

void RndButton::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new Button");
        return;
    }
    ReleaseRefs();
    int nState;
    stream.ReadEndian(&nState, sizeof(nState));
    mState = nState;
    mMesh = FindByName<RndMesh>(stream);
    mText = FindByName<RndText>(stream);
    stream >> mMats >> mFonts;
    AcquireRefs();
}

void RndButton::AcquireRefs() {
    if (mMesh != nullptr) {
        mMesh->AddRef(this);
    }
    if (mText != nullptr) {
        mText->AddRef(this);
    }
    for (RndMat *pMat : mMats) {
        if (pMat != nullptr) {
            pMat->AddRef(this);
        }
    }
    for (RndFont *pFont : mFonts) {
        if (pFont != nullptr) {
            pFont->AddRef(this);
        }
    }
}

void RndButton::ReleaseRefs() {
    if (mMesh != nullptr) {
        mMesh->RemoveRef(this);
    }
    if (mText != nullptr) {
        mText->RemoveRef(this);
    }
    for (RndMat *pMat : mMats) {
        if (pMat != nullptr) {
            pMat->RemoveRef(this);
        }
    }
    for (RndFont *pFont : mFonts) {
        if (pFont != nullptr) {
            pFont->RemoveRef(this);
        }
    }
}

void RndButton::Replace(RndObject *pFrom, RndObject *pTo) {
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
    ReplaceEntries(this, mMats, pFrom, pTo);
    ReplaceEntries(this, mFonts, pFrom, pTo);
}
