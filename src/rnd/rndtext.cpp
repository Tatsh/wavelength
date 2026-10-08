#include "rnd/rndtext.h"

#include <cmath>
#include <cstring>

#include "math/frustum.h"
#include "math/sphere.h"
#include "math/vector2.h"
#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndcursor.h"
#include "rnd/rndmanager.h"
#include "rnd/rndrenderer.h"

namespace {

// The first version with each later field.
constexpr int kRevColor = 1;
constexpr int kRevTransformable = 2;
constexpr int kRevAlignBits = 3;
constexpr int kRevWordWrap = 4;
constexpr int kRevDepthTest = 5;
constexpr int kRevText = 5;

// The alignment each legacy alignment index stands for.
const int kOldAligns[] = {
    RndText::kAlignTop | RndText::kAlignLeft,
    RndText::kAlignTop | RndText::kAlignCenter,
    RndText::kAlignTop | RndText::kAlignRight,
    RndText::kAlignBottom | RndText::kAlignLeft,
    RndText::kAlignBottom | RndText::kAlignCenter,
    RndText::kAlignBottom | RndText::kAlignRight,
};

// The depth a legacy position's second coordinate is scaled to.
constexpr float kLegacyDepthScale = 0.75f;

constexpr float kDefaultWrapWidth = 100.0f;
constexpr float kMaxLegacyWrapWidth = 1000.0f;

// The longest text the mesh shows, and the spaces a tab becomes.
constexpr int kMaxTextLength = 800;
const char *const kTabSpaces = "   ";

// The quads, triangles, and vertices of a character.
constexpr int kFacesPerChar = 2;
constexpr int kVertsPerChar = 4;

template <typename T>
T *FindByName(BinStream &stream) {
    String name;
    stream >> name;
    if (name.mLength == 0) {
        return nullptr;
    }
    return dynamic_cast<T *>(TheManager.Find(name.c_str()));
}

void WriteFlag(BinStream &stream, int bFlag) {
    const char nFlag = static_cast<char>(bFlag);
    stream.Write(&nFlag, sizeof(nFlag));
}

int ReadFlag(BinStream &stream) {
    unsigned char nFlag;
    stream.Read(&nFlag, sizeof(nFlag));
    return nFlag != 0;
}

void MultiplyPoint(const Transform &xfm, const Vector3 &v, Vector3 &out) {
    const float fX =
        xfm.mBasisX.x * v.x + xfm.mBasisY.x * v.y + xfm.mBasisZ.x * v.z + xfm.mTranslation.x;
    const float fY =
        xfm.mBasisX.y * v.x + xfm.mBasisY.y * v.y + xfm.mBasisZ.y * v.z + xfm.mTranslation.y;
    const float fZ =
        xfm.mBasisX.z * v.x + xfm.mBasisY.z * v.y + xfm.mBasisZ.z * v.z + xfm.mTranslation.z;
    out.x = fX;
    out.y = fY;
    out.z = fZ;
}

} // namespace

const char *RndText::sClassName = "Text";
int RndText::sRev = 6;

RndText::RndText(const char *pszName) : RndObject(pszName) {
    mColor.r = 1.0f;
    mColor.a = 1.0f;
    mColor.g = 1.0f;
    mColor.b = 1.0f;
    mAlign = kAlignTop | kAlignLeft;
    mWrapWidth = kDefaultWrapWidth;
    mFont = nullptr;
    mWordWrap = 0;
    mMesh = nullptr;
    mDepthTest = 0;
}

RndText::~RndText() {
    ReleaseRefs();
}

void RndText::ListDrawObjects(std::list<RndObject *> &objects) {
    if (mMesh != nullptr) {
        objects.push_back(mMesh->mMat);
    }
}

void RndText::ListDrawables(std::list<RndDrawable *> &drawables) {
    if (mMesh != nullptr && !mMesh->mGeomOwner->mVerts.empty()) {
        const std::vector<RndMesh::Vert> &verts = mMesh->mGeomOwner->mVerts;
        const Vector3 &first = verts.front().mPos;
        const Vector3 &last = verts.back().mPos;
        Sphere sphere;
        sphere.mCenter.z = (first.z + last.z) * 0.5f;
        sphere.mCenter.x = (first.x + last.x) * 0.5f;
        sphere.mCenter.y = (first.y + last.y) * 0.5f;
        const float fX = first.x - sphere.mCenter.x;
        const float fY = first.y - sphere.mCenter.y;
        const float fZ = first.z - sphere.mCenter.z;
        sphere.mRadius = std::sqrt(fX * fX + fY * fY + fZ * fZ);
        MultiplyPoint(mMesh->mWorldXfm, sphere.mCenter, sphere.mCenter);
        if (IsSphereOutsideFrustum(sphere, RndCam::sCurrent->mWorldFrustum) == 0) {
            drawables.push_back(this);
        }
    }
    RndDrawable::ListDrawables(drawables);
}

void RndText::SetShowing(int nShowing) {
    if (nShowing == mShowing) {
        return;
    }
    mShowing = nShowing;
    if (nShowing != 0) {
        BuildMesh();
    } else {
        delete mMesh;
        mMesh = nullptr;
    }
}

void RndText::SetHighlight(int nHighlight) {
    RndDrawable::SetHighlight(nHighlight);
    if (mMesh != nullptr) {
        mMesh->SetHighlight(nHighlight);
    }
}

int RndText::DrawShowing() {
    if (mMesh != nullptr) {
        mMesh->Draw();
        return 1;
    }
    const Vector2 charSize{0.0f, 0.0f};
    const Vector2 pos{mLocalXfm.mTranslation.x * static_cast<float>(TheRnd->mScreenWidth),
                      mLocalXfm.mTranslation.y * static_cast<float>(TheRnd->mScreenHeight)};
    TheRnd->DrawString(mText.c_str(), pos, charSize, mColor);
    return 1;
}

void RndText::SetColor(const Color &color) {
    mColor = color;
    BuildMesh();
}

void RndText::SetAlign(int nAlign) {
    mAlign = nAlign;
    SyncText();
}

void RndText::SetText(const char *pszText) {
    mPreWrapText = pszText;
    SyncText();
}

void RndText::SetFont(RndFont *pFont) {
    if (mFont != nullptr) {
        mFont->RemoveRef(this);
    }
    mFont = pFont;
    if (pFont != nullptr) {
        pFont->AddRef(this);
    }
    SyncText();
}

void RndText::Collide(const Segment &segment, std::list<Collision> &collisions) {
    if (mShowing == 0) {
        return;
    }
    if (mMesh != nullptr) {
        std::list<Collision> meshCollisions;
        mMesh->Collide(segment, meshCollisions);
        for (Collision &collision : meshCollisions) {
            collision.mObject = this;
        }
        collisions.splice(collisions.end(), meshCollisions);
    }
    RndCollideable::Collide(segment, collisions);
}

void RndText::SetBillboard(int nBillboard) {
    RndTransformable::SetBillboard(nBillboard);
    if (mMesh != nullptr) {
        mMesh->SetBillboard(nBillboard);
    }
}

int RndText::UpdateWorldXfm(RndTransformable *pParent, int bForce) {
    const int nResult = RndTransformable::UpdateWorldXfm(pParent, bForce);
    if (mMesh != nullptr) {
        mMesh->UpdateWorldXfm(this, nResult);
    }
    return nResult;
}

void RndText::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndCollideable::DumpText(stream);
    RndTransformable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndText]\n";
    stream << "font:" << static_cast<const RndObject *>(mFont)
           << " align:" << static_cast<Align>(mAlign) << "\n";
    stream << "preWrapText:";
    stream.Print(mPreWrapText.c_str());
    stream << "\n";
    stream << "color:" << mColor << " word wrap:" << (mWordWrap != 0)
           << " wrap width:" << mWrapWidth << "\n";
    if (stream.mDumpLevel < 2) {
        return;
    }
    stream << "text:";
    stream.Print(mText.c_str());
    stream << "mesh:" << static_cast<const RndObject *>(mMesh) << "\n";
}

void RndText::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndDrawable::Save(stream);
    RndCollideable::Save(stream);
    RndTransformable::Save(stream);
    stream.WriteString(mFont != nullptr ? mFont->mName.c_str() : "");
    stream.WriteEndian(&mAlign, sizeof(mAlign));
    stream.WriteString(mPreWrapText.c_str());
    stream.WriteEndian(&mColor.r, sizeof(mColor.r));
    stream.WriteEndian(&mColor.g, sizeof(mColor.g));
    stream.WriteEndian(&mColor.b, sizeof(mColor.b));
    stream.WriteEndian(&mColor.a, sizeof(mColor.a));
    WriteFlag(stream, mWordWrap);
    stream.WriteEndian(&mWrapWidth, sizeof(mWrapWidth));
    WriteFlag(stream, mDepthTest);
}

void RndText::Replace(RndObject *pFrom, RndObject *pTo) {
    RndDrawable::Replace(pFrom, pTo);
    RndCollideable::Replace(pFrom, pTo);
    RndTransformable::Replace(pFrom, pTo);
    if (mFont != pFrom) {
        return;
    }
    if (mFont != nullptr) {
        mFont->RemoveRef(this);
    }
    if (mFont != nullptr) {
        mFont = pTo != nullptr ? dynamic_cast<RndFont *>(pTo) : nullptr;
    }
    if (mFont != nullptr) {
        mFont->AddRef(this);
    }
}

void RndText::Copy(const RndObject *pSource, int nFlags) {
    const RndText *pText = pSource != nullptr ? dynamic_cast<const RndText *>(pSource) : nullptr;
    RndDrawable::Copy(pSource, nFlags);
    RndCollideable::Copy(pSource, nFlags);
    RndTransformable::Copy(pSource, nFlags);
    ReleaseRefs();
    mFont = pText->mFont;
    mAlign = pText->mAlign;
    mPreWrapText = pText->mPreWrapText;
    mWordWrap = pText->mWordWrap;
    mWrapWidth = pText->mWrapWidth;
    mDepthTest = pText->mDepthTest;
    mColor = pText->mColor;
    AcquireRefs();
}

void RndText::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Text");
        return;
    }
    RndDrawable::Load(stream);
    RndCollideable::Load(stream);
    if (nRev >= kRevTransformable) {
        RndTransformable::Load(stream);
    }
    ReleaseRefs();
    mFont = FindByName<RndFont>(stream);
    if (nRev < kRevAlignBits) {
        int nOldAlign;
        stream.ReadEndian(&nOldAlign, sizeof(nOldAlign));
        mAlign = kOldAligns[nOldAlign];
    } else {
        int nAlign;
        stream.ReadEndian(&nAlign, sizeof(nAlign));
        mAlign = nAlign;
    }
    if (nRev < kRevTransformable) {
        float fX;
        stream.ReadEndian(&fX, sizeof(fX));
        float fY;
        stream.ReadEndian(&fY, sizeof(fY));
        mDirty = 1;
        mLocalXfm.mTranslation.x = fX;
        mLocalXfm.mTranslation.y = 0.0f;
        mLocalXfm.mTranslation.z = -fY * kLegacyDepthScale;
    }
    stream >> mPreWrapText;
    if (nRev >= kRevColor) {
        stream.ReadEndian(&mColor.r, sizeof(mColor.r));
        stream.ReadEndian(&mColor.g, sizeof(mColor.g));
        stream.ReadEndian(&mColor.b, sizeof(mColor.b));
        stream.ReadEndian(&mColor.a, sizeof(mColor.a));
    }
    if (nRev < kRevWordWrap) {
        mWordWrap = 0;
        mWrapWidth = kDefaultWrapWidth;
    } else {
        mWordWrap = ReadFlag(stream);
        stream.ReadEndian(&mWrapWidth, sizeof(mWrapWidth));
        if (nRev < kRevDepthTest) {
            if (mWrapWidth < 0.0f) {
                mWrapWidth = kDefaultWrapWidth;
            } else if (kMaxLegacyWrapWidth < mWrapWidth) {
                mWrapWidth = kMaxLegacyWrapWidth;
            }
        }
    }
    if (nRev == kRevText) {
        stream >> mText;
    }
    if (nRev >= kRevDepthTest) {
        mDepthTest = ReadFlag(stream);
    }
    AcquireRefs();
}

void RndText::UpdateCursors() {
    for (auto it = mRefs.begin(); it != mRefs.end();) {
        RndObject *pRef = *it;
        ++it;
        RndCursor *pCursor = pRef != nullptr ? dynamic_cast<RndCursor *>(pRef) : nullptr;
        if (pCursor != nullptr) {
            pCursor->SetText(this);
        }
    }
}

void RndText::SyncText() {
    mText = mPreWrapText;
    int nPos = 0;
    while ((nPos = mText.Find('\t', nPos)) != String::npos) {
        const String spaces(kTabSpaces);
        mText.Replace(nPos, 1, spaces);
    }
    if (mText.mLength > kMaxTextLength) {
        mText.Truncate(kMaxTextLength);
    }
    if (mWordWrap != 0 && mFont != nullptr) {
        WrapText(mText.c_str(), mText);
    }
    BuildMesh();
}

void RndText::SetWordWrap(int nWordWrap) {
    mWordWrap = nWordWrap;
    SyncText();
}

void RndText::SetWrapWidth(float fWrapWidth) {
    mWrapWidth = fWrapWidth;
    SyncText();
}

void RndText::SyncDepthTest() {
    if (mMesh == nullptr) {
        return;
    }
    if (mDepthTest != 0) {
        mMesh->mZFunc = RndMesh::kZFuncLess;
        mMesh->mZMode = RndMesh::kZModeZReadOnly;
    } else {
        mMesh->mZFunc = RndMesh::kZFuncNever;
        mMesh->mZMode = RndMesh::kZModeDisable;
    }
}

void RndText::SetDepthTest(int nDepthTest) {
    mDepthTest = nDepthTest;
    SyncDepthTest();
}

float RndText::MeasureWidth(const char *pszText, int nCount) {
    float fWidth = 0.0f;
    if (mFont == nullptr) {
        return fWidth;
    }
    for (int i = 0; i < nCount; ++i) {
        fWidth += mFont->CharWidth(pszText[i]);
    }
    return fWidth;
}

void RndText::WrapText(const char *pszText, String &wrapped) {
    const int nLength = static_cast<int>(std::strlen(pszText));
    const char *pEnd = pszText + nLength;
    const int nBufferSize = nLength * 2 + 2;
    auto *pBuffer = static_cast<char *>(PoolMemAlloc(nBufferSize, "RndText", 0));
    std::memset(pBuffer, 0, nBufferSize);
    char *pOut = pBuffer;
    const char *pSpace = nullptr;
    char *pSpaceOut = nullptr;
    float fWidth = 0.0f;
    for (const char *p = pszText; p < pEnd; ++p, ++pOut) {
        *pOut = *p;
        if (*p == '\n') {
            fWidth = 0.0f;
            pSpace = nullptr;
            pSpaceOut = nullptr;
        } else if (*p == ' ') {
            pSpace = p;
            pSpaceOut = pOut;
        }
        fWidth += mFont != nullptr ? mFont->CharWidth(*p) : 0.0f;
        if (mWrapWidth < fWidth) {
            if (pSpace != nullptr) {
                p = pSpace;
                pOut = pSpaceOut;
            }
            ++pOut;
            fWidth = 0.0f;
            *pOut = '\n';
            pSpace = nullptr;
            pSpaceOut = nullptr;
        }
    }
    *pOut = '\0';
    wrapped = pBuffer;
    PoolMemFree(pBuffer);
}

int RndText::NumLines() {
    int nLines = 0;
    int nPos = 0;
    while ((nPos = mText.Find('\n', nPos)) != String::npos) {
        ++nLines;
        ++nPos;
    }
    return nLines + 1;
}

const char *RndText::MeasureLine(const char *pszLine, float &fWidth) {
    fWidth = 0.0f;
    const char *pEnd = mText.c_str() + mText.mLength;
    const char *p = pszLine;
    while (p < pEnd && *p != '\n') {
        fWidth += mFont->CharWidth(*p) + mFont->mSpace;
        ++p;
    }
    return p + 1;
}

void RndText::BuildMesh() {
    delete mMesh;
    mMesh = nullptr;
    if (mShowing == 0 || mFont == nullptr || mPreWrapText.mLength == 0) {
        return;
    }
    const char *pszMeshName = FormatString("[%s_mesh]", mName.c_str());
    RndObject *pObject = TheManager.Create(RndMesh::sClassName, pszMeshName);
    mMesh = pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr;
    const int nLines = NumLines();
    mMesh->SetBillboard(mBillboard);
    mMesh->mInternal = 1;
    mMesh->SetMat(mFont->mMat);
    RndMesh *pGeom = mMesh->mGeomOwner;
    pGeom->mFaces.resize((mText.mLength + 1 - nLines) * kFacesPerChar, RndMesh::Face());
    pGeom = mMesh->mGeomOwner;
    pGeom->mVerts.resize(pGeom->mFaces.size() * kVertsPerChar / kFacesPerChar, RndMesh::Vert());
    float fLine = 0.0f;
    SyncDepthTest();
    if ((mAlign & kAlignMiddle) != 0) {
        fLine = static_cast<float>(-nLines) * 0.5f;
    } else if ((mAlign & kAlignBottom) != 0) {
        fLine = static_cast<float>(-nLines);
    }
    const char *pStart = mText.c_str();
    const char *p = pStart;
    int nFirstChar = 0;
    float fWidth = 0.0f;
    for (; p != mText.c_str() + mText.mLength; ++p) {
        if (*p == '\n') {
            BuildLine(nFirstChar, pStart, p, fLine, fWidth);
            fLine += 1.0f;
            nFirstChar += static_cast<int>(p - pStart);
            fWidth = 0.0f;
            pStart = p + 1;
        } else {
            fWidth += mFont->CharWidth(*p) + mFont->mSpace;
        }
    }
    BuildLine(nFirstChar, pStart, p, fLine, fWidth);
    mMesh->Sync(RndMesh::kSyncAll);
    mMesh->UpdateWorldXfm(this, 1);
}

void RndText::BuildLine(
    int nFirstChar, const char *pszStart, const char *pszEnd, float fLine, float fWidth) {
    float fX = 0.0f;
    if ((mAlign & kAlignCenter) != 0) {
        fX = -fWidth * 0.5f;
    } else if ((mAlign & kAlignRight) != 0) {
        fX = -fWidth;
    }
    RndMesh *pGeom = mMesh->mGeomOwner;
    const float fZ = -fLine * mFont->mSize;
    int nFace = nFirstChar * kFacesPerChar;
    int nVert = nFirstChar * kVertsPerChar;
    for (const char *p = pszStart; p != pszEnd; ++p) {
        Vector2 uvStart;
        Vector2 uvEnd;
        mFont->CharUv(*p, uvStart, uvEnd);
        RndMesh::Vert &topLeft = pGeom->mVerts[nVert];
        RndMesh::Vert &bottomLeft = pGeom->mVerts[nVert + 1];
        RndMesh::Vert &bottomRight = pGeom->mVerts[nVert + 2];
        RndMesh::Vert &topRight = pGeom->mVerts[nVert + 3];
        topLeft.mTex = uvStart;
        bottomLeft.mTex.x = uvStart.x;
        bottomLeft.mTex.y = uvEnd.y;
        bottomRight.mTex = uvEnd;
        topRight.mTex.y = uvStart.y;
        topRight.mTex.x = uvEnd.x;
        const float fAdvance = mFont->CharWidth(*p);
        const float fRight = fX + fAdvance;
        const float fBottom = fZ - mFont->mSize;
        topLeft.mPos.x = fX;
        bottomLeft.mPos.x = fX;
        topLeft.mPos.y = 0.0f;
        topLeft.mPos.z = fZ;
        bottomRight.mPos.z = fBottom;
        bottomLeft.mPos.y = 0.0f;
        bottomLeft.mPos.z = fBottom;
        bottomRight.mPos.x = fRight;
        bottomRight.mPos.y = 0.0f;
        topRight.mPos.x = fRight;
        topRight.mPos.y = 0.0f;
        topRight.mPos.z = fZ;
        topLeft.mColor = mColor;
        topRight.mColor = mColor;
        bottomRight.mColor = mColor;
        bottomLeft.mColor = mColor;
        const auto nIndex = static_cast<unsigned short>(nVert);
        RndMesh::Face &first = pGeom->mFaces[nFace];
        RndMesh::Face &second = pGeom->mFaces[nFace + 1];
        first.mVerts[1] = nIndex + 1;
        second.mVerts[0] = nIndex;
        second.mVerts[1] = nIndex + 2;
        second.mVerts[2] = nIndex + 3;
        first.mVerts[0] = nIndex;
        first.mVerts[2] = nIndex + 2;
        nFace += kFacesPerChar;
        nVert += kVertsPerChar;
        fX += fAdvance + mFont->mSpace;
    }
}

Vector3 RndText::CharPosition(int nIndex) {
    if (mFont == nullptr) {
        return Vector3{0.0f, 0.0f, 0.0f, 1.0f};
    }
    if (mWordWrap != 0) {
        const char *pPreWrap = mPreWrapText.c_str();
        const char *pWrapped = mText.c_str();
        for (int n = nIndex; n > 0; ++pWrapped) {
            if (*pWrapped == *pPreWrap) {
                --n;
                ++pPreWrap;
            }
        }
        if (*pWrapped != *pPreWrap) {
            ++pWrapped;
        }
        nIndex = static_cast<int>(pWrapped - mText.c_str());
    }
    const char *pEnd = mText.c_str() + nIndex;
    const char *pLine = mText.c_str();
    float fLine = 0.0f;
    float fWidth;
    const char *pNext = MeasureLine(pLine, fWidth);
    while (pEnd < pLine || !(pEnd < pNext)) {
        pLine = pNext;
        fLine += 1.0f;
        pNext = MeasureLine(pLine, fWidth);
    }
    float fX = 0.0f;
    for (const char *p = pLine; p < pEnd && p < pNext; ++p) {
        fX += mFont->CharWidth(*p) + mFont->mSpace;
    }
    if ((mAlign & kAlignCenter) != 0) {
        fX -= fWidth * 0.5f;
    } else if ((mAlign & kAlignRight) != 0) {
        fX -= fWidth;
    }
    const auto fLines = static_cast<float>(NumLines());
    if ((mAlign & kAlignMiddle) != 0) {
        fLine -= (fLines - 1.0f) * 0.5f;
    } else if ((mAlign & kAlignBottom) != 0) {
        fLine -= fLines - 1.0f;
    }
    return Vector3{fX, 0.0f, -fLine * mFont->mSize, 1.0f};
}

void RndText::ReleaseRefs() {
    if (mFont != nullptr) {
        mFont->RemoveRef(this);
    }
    delete mMesh;
    mMesh = nullptr;
}

void RndText::AcquireRefs() {
    if (mFont != nullptr) {
        mFont->AddRef(this);
    }
    SyncText();
}

PrnStream &operator<<(PrnStream &stream, RndText::Align eAlign) {
    const int nAlign = eAlign;
    if ((nAlign & RndText::kAlignTop) != 0) {
        stream << "Top";
    } else if ((nAlign & RndText::kAlignMiddle) != 0) {
        stream << "Middle";
    } else if ((nAlign & RndText::kAlignBottom) != 0) {
        stream << "Bottom";
    }
    if ((nAlign & RndText::kAlignLeft) != 0) {
        stream << "Left";
    } else if ((nAlign & RndText::kAlignCenter) != 0) {
        stream << "Center";
    } else if ((nAlign & RndText::kAlignRight) != 0) {
        stream << "Right";
    }
    return stream;
}
