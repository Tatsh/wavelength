#include "rnd/rndcursor.h"

#include <cmath>
#include <cstring>

#include "os/debug.h"
#include "os/string.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"

namespace {

// The defaults the constructor sets.
constexpr float kDefaultOffset = -0.1f;
constexpr float kDefaultStutter = 0.8f;

constexpr int kDumpInternals = 2;
constexpr float kHalf = 0.5f;

// The mask is a quad of four vertices and two triangles over the line.
enum MaskVert {
    kMaskTopLeft = 0,
    kMaskBottomLeft = 1,
    kMaskTopRight = 2,
    kMaskBottomRight = 3,
    kMaskVertCount = 4,
};
constexpr int kMaskFaceCount = 2;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.GetLength() == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

void WriteObjectName(BinStream &stream, const RndObject *pObject) {
    stream.WriteString(pObject != nullptr ? pObject->GetName() : "");
}

} // namespace

const char *RndCursor::sClassName = "Cursor";
int RndCursor::sRev = 0;
int RndCursor::sLoadRev;

RndCursor::RndCursor(const char *pszName) : RndObject(pszName) {
    mOffset = kDefaultOffset;
    mStutter = kDefaultStutter;
    mText = nullptr;
    mBlur = nullptr;
    mCursor = nullptr;
    mMask = nullptr;
    mStartX = 0.0f;
    mCenterZ = 0.0f;
    AcquireRefs();
}

RndCursor::~RndCursor() {
    ReleaseRefs();
}

void RndCursor::Copy(const RndObject *pSource, int nFlags) {
    const RndCursor *pCursor =
        pSource != nullptr ? dynamic_cast<const RndCursor *>(pSource) : nullptr;
    RndAnimatable::Copy(pSource, nFlags);
    RndDrawable::Copy(pSource, nFlags);
    ReleaseRefs();
    mText = pCursor->mText;
    mBlur = pCursor->mBlur;
    mOffset = pCursor->mOffset;
    mStutter = pCursor->mStutter;
    AcquireRefs();
}

void RndCursor::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndAnimatable::DumpText(stream);
    RndDrawable::DumpText(stream);
    if (stream.GetDumpLevel() <= 0) {
        return;
    }
    stream << "[RndCursor]\n";
    const RndObject *pText = mText;
    const RndObject *pBlur = mBlur;
    stream << "text:" << pText << " blur:" << pBlur << " offset:" << mOffset << "\n";
    stream << "stutter:" << mStutter << "\n";
    if (stream.GetDumpLevel() < kDumpInternals) {
        return;
    }
    const RndObject *pMask = mMask.get();
    const RndObject *pCursor = mCursor.get();
    stream << "mask:" << pMask << "\n";
    stream << "cursor:" << pCursor << "\n";
}

void RndCursor::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndAnimatable::Save(stream);
    RndDrawable::Save(stream);
    WriteObjectName(stream, mText);
    WriteObjectName(stream, mBlur);
    float fValue = mOffset;
    stream.WriteEndian(&fValue, sizeof(fValue));
    fValue = mStutter;
    stream.WriteEndian(&fValue, sizeof(fValue));
}

void RndCursor::Load(BinStream &stream) {
    stream.ReadEndian(&sLoadRev, sizeof(sLoadRev));
    if (sLoadRev > sRev) {
        DebugNotify("Can't load new Cursor");
        return;
    }
    RndAnimatable::Load(stream);
    RndDrawable::Load(stream);
    ReleaseRefs();
    mText = FindByName<RndText>(stream);
    mBlur = FindByName<RndBlur>(stream);
    stream.ReadEndian(&mOffset, sizeof(mOffset));
    stream.ReadEndian(&mStutter, sizeof(mStutter));
    AcquireRefs();
}

void RndCursor::AcquireRefs() {
    if (mText != nullptr) {
        mText->AddRef(this);
    }
    if (mBlur != nullptr) {
        mBlur->AddRef(this);
    }
    BuildMask();
    BuildCursor();
}

void RndCursor::ReleaseRefs() {
    if (mText != nullptr) {
        mText->RemoveRef(this);
    }
    if (mBlur != nullptr) {
        mBlur->RemoveRef(this);
    }
    mMask.reset();
    mCursor.reset();
}

void RndCursor::BuildCursor() {
    mCursor.reset();
    if (mBlur == nullptr || mBlur->GetMesh() == nullptr) {
        return;
    }
    RndObject *pObject =
        TheManager.Create(RndBlur::sClassName, FormatString("[%s_cursor]", mName.c_str()));
    mCursor.reset(pObject != nullptr ? dynamic_cast<RndBlur *>(pObject) : nullptr);
    mCursor->SetInternal(1);
    mCursor->Copy(mBlur, 0);
    RndMesh *pMesh = mCursor->GetMesh();
    pMesh->SetZFunc(1);
    pMesh->SetZMode(0);
}

void RndCursor::BuildMask() {
    mMask.reset();
    if (mText == nullptr || mText->GetFont() == nullptr) {
        return;
    }
    mText->SetDepthTest(1);
    if (mText->NumLines() != 1) {
        DebugNotify("%s must be one line", mText->GetName());
        return;
    }
    RndObject *pObject =
        TheManager.Create(RndMesh::sClassName, FormatString("[%s_mask]", mName.c_str()));
    mMask.reset(pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr);
    mMask->SetInternal(1);

    Vector3 start = mText->CharPosition(0);
    Vector3 end =
        mText->CharPosition(static_cast<int>(std::strlen(mText->GetPreWrapText().c_str())));
    const int nAlign = mText->GetAlign();
    const float fSize = mText->GetFont()->GetSize();
    if ((nAlign & RndText::kAlignMiddle) != 0) {
        const float fHalf = fSize * kHalf;
        start.z += fHalf;
        end.z -= fHalf;
    } else if ((nAlign & RndText::kAlignTop) != 0) {
        end.z -= fSize;
    } else {
        start.z += fSize;
    }
    mStartX = start.x;
    mWidth = end.x - start.x;
    mCenterZ = (start.z + end.z) * kHalf;

    mMask->GetGeomOwner()->GetVerts().resize(kMaskVertCount);
    std::vector<RndMesh::Vert> &verts = mMask->GetGeomOwner()->GetVerts();
    verts[kMaskTopLeft].mPos = start;
    verts[kMaskBottomLeft].mPos.x = start.x;
    verts[kMaskBottomLeft].mPos.z = end.z;
    verts[kMaskBottomLeft].mPos.y = 0.0f;
    verts[kMaskTopRight].mPos.x = end.x;
    verts[kMaskTopRight].mPos.z = start.z;
    verts[kMaskTopRight].mPos.y = 0.0f;
    verts[kMaskBottomRight].mPos = end;
    for (int i = kMaskTopLeft; i < kMaskVertCount; ++i) {
        verts[i].mColor = Color{0.0f, 0.0f, 0.0f, 0.0f};
    }

    mMask->GetGeomOwner()->GetFaces().resize(kMaskFaceCount);
    std::vector<RndMesh::Face> &faces = mMask->GetGeomOwner()->GetFaces();
    faces[0].mVerts[0] = kMaskTopLeft;
    faces[0].mVerts[1] = kMaskBottomLeft;
    faces[0].mVerts[2] = kMaskTopRight;
    faces[1].mVerts[0] = kMaskTopRight;
    faces[1].mVerts[1] = kMaskBottomLeft;
    faces[1].mVerts[2] = kMaskBottomRight;
    mMask->Sync(RndMesh::kSyncAll);
}

void RndCursor::Replace(RndObject *pFrom, RndObject *pTo) {
    RndAnimatable::Replace(pFrom, pTo);
    RndDrawable::Replace(pFrom, pTo);
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
        BuildMask();
    }
    if (static_cast<RndObject *>(mBlur) == pFrom) {
        if (pFrom != nullptr) {
            pFrom->RemoveRef(this);
        }
        if (mBlur != nullptr) {
            mBlur = pTo != nullptr ? dynamic_cast<RndBlur *>(pTo) : nullptr;
        }
        if (mBlur != nullptr) {
            mBlur->AddRef(this);
        }
        BuildCursor();
    }
}

float RndCursor::CursorPos() {
    const float fFrame = mFrame;
    const float fChar = std::floor(fFrame);
    if (fFrame <= fChar + mStutter) {
        return fChar;
    }
    return fChar + ((fFrame - (fChar + mStutter)) / (1.0f - mStutter));
}

float RndCursor::EndFrame() {
    return static_cast<float>(std::strlen(mText->GetPreWrapText().c_str())) + mStutter;
}

int RndCursor::DrawShowing() {
    if (mText == nullptr || mText->GetShowing() == 0) {
        return 1;
    }
    const int nLength = static_cast<int>(std::strlen(mText->GetPreWrapText().c_str()));
    float fX = (mWidth * CursorPos()) / static_cast<float>(nLength);
    if (mWidth < fX) {
        fX = mWidth;
    } else if (fX < 0.0f) {
        fX = 0.0f;
    }

    if (mMask != nullptr) {
        // The binary also copies an uninitialised fourth word into the translation.
        Vector3 &translation = mMask->GetLocalXfm().mTranslation;
        mMask->SetDirty(1);
        translation.x = fX;
        translation.y = mOffset;
        translation.z = 0.0f;
        mMask->UpdateWorldXfm(mText, 0);
        mMask->Draw();
    }
    mText->Draw();

    if (mCursor == nullptr) {
        return 1;
    }
    RndMesh *pMesh = mCursor->GetMesh();
    if (pMesh == nullptr) {
        return 1;
    }
    // The binary also copies an uninitialised fourth word into the translation.
    Transform xfm = pMesh->GetLocalXfm();
    xfm.mTranslation.x = fX + mStartX;
    xfm.mTranslation.y = 0.0f;
    xfm.mTranslation.z = mCenterZ;
    const float fFrame = mFrame;
    float fScale;
    if (fFrame < 0.0f || static_cast<float>(nLength) + mStutter < fFrame) {
        fScale = 0.0f;
    } else {
        fScale = 1.0f;
    }
    xfm.mBasisX.x = fScale;
    xfm.mBasisX.y = 0.0f;
    xfm.mBasisX.z = 0.0f;
    pMesh->GetLocalXfm() = xfm;
    pMesh->SetDirty(1);
    pMesh->UpdateWorldXfm(mText, 0);
    mCursor->Draw();
    return 1;
}

void RndCursor::ListDrawObjects(std::list<RndObject *> &objects) {
    if (mBlur != nullptr) {
        mBlur->ListDrawObjects(objects);
    }
    if (mText != nullptr) {
        mText->ListDrawObjects(objects);
    }
}

void RndCursor::ListDrawables(std::list<RndDrawable *> &drawables) {
    if (mText != nullptr) {
        mText->ListDrawables(drawables);
    }
    RndDrawable::ListDrawables(drawables);
}

void RndCursor::SetText(RndText *pText) {
    if (mText != nullptr) {
        mText->RemoveRef(this);
    }
    mText = pText;
    if (pText != nullptr) {
        pText->AddRef(this);
    }
    BuildMask();
}
