#include "rnd/rndline.h"

#include <cmath>

#include "math/ray.h"
#include "math/transform.h"
#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"

namespace {

// The first version with the fold angle and the caps, and the first with line pairs.
constexpr int kRevFold = 1;
constexpr int kRevLinePairs = 2;

constexpr float kDefaultWidth = 1.0f;
constexpr float kDefaultFoldAngle = 1.57079637f;

// How far in front of the near plane a point has to be to be drawn.
constexpr float kNearMargin = 0.01f;

// The cosine above which consecutive segments are treated as straight.
constexpr float kStraightCos = 0.99985f;

// The mesh parts SetNumPoints() and SetPointColor() rebuild.
constexpr int kSyncPointLayout =
    RndMesh::kSyncColors | RndMesh::kSyncTexCoords | RndMesh::kSyncFaces;

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

// Move a camera-space point across the screen plane by a screen offset.
// NTSC-U/C: 0x0022a0b8, PAL: 0x00232e28
void AddScreen(const Vector3 &point, const Vector2 &offset, Vector3 &out) {
    const float fX = point.x + offset.x;
    const float fZ = point.z + offset.y;
    out.y = point.y;
    out.x = fX;
    out.z = fZ;
}

// NTSC-U/C: 0x0022a0e8, PAL: 0x00232e58
void SubtractScreen(const Vector3 &point, const Vector2 &offset, Vector3 &out) {
    const float fX = point.x - offset.x;
    const float fZ = point.z - offset.y;
    out.y = point.y;
    out.x = fX;
    out.z = fZ;
}

// Move a point clipped by the near plane to the plane, along the segment to a visible point.
void ClipToNear(Vector3 &clipped, const Vector3 &visible, float fRatio) {
    if (fRatio == 0.0f) {
        return;
    }
    if (fRatio == 1.0f) {
        clipped = visible;
        return;
    }
    const float fInverse = 1.0f - fRatio;
    clipped.x = visible.x * fRatio + clipped.x * fInverse;
    clipped.y = visible.y * fRatio + clipped.y * fInverse;
    clipped.z = visible.z * fRatio + clipped.z * fInverse;
}

void Project(RndLine::Point &point) {
    const float fInverse = 1.0f / point.mCamPoint.y;
    point.mScreen.x = point.mCamPoint.x * fInverse;
    point.mScreen.y = point.mCamPoint.z * fInverse;
}

// Set the direction to the next point, of unit length or zero, and the offset of the edges.
void SetDirection(RndLine::Point &point, const Vector2 &next, float fWidth) {
    point.mDir.x = next.x - point.mScreen.x;
    point.mDir.y = next.y - point.mScreen.y;
    if (point.mDir.x == 0.0f && point.mDir.y == 0.0f) {
        point.mDir.x = 0.0f;
        point.mDir.y = 0.0f;
    } else {
        const float fScale =
            1.0f / std::sqrt(point.mDir.x * point.mDir.x + point.mDir.y * point.mDir.y);
        point.mDir.x *= fScale;
        point.mDir.y *= fScale;
    }
    point.mOffset.y = point.mDir.x;
    point.mOffset.x = -point.mDir.y;
    point.mOffset.y *= fWidth;
    point.mOffset.x *= fWidth;
}

void FillPair(const RndLine::Point &point, RndMesh::Vert *&pVert) {
    SubtractScreen(point.mCamPoint, point.mOffset, pVert->mPos);
    ++pVert;
    AddScreen(point.mCamPoint, point.mOffset, pVert->mPos);
    ++pVert;
}

void FillCap(const RndLine::Point &point, const Vector2 &cap, RndMesh::Vert *&pVert) {
    SubtractScreen(point.mCamPoint, point.mOffset, pVert->mPos);
    AddScreen(pVert->mPos, cap, pVert->mPos);
    ++pVert;
    AddScreen(point.mCamPoint, point.mOffset, pVert->mPos);
    AddScreen(pVert->mPos, cap, pVert->mPos);
    ++pVert;
}

} // namespace

const char *RndLine::sClassName = "Line";
int RndLine::sRev = 2;

RndLine::RndLine(const char *pszName) : RndObject(pszName) {
    mWidth = kDefaultWidth;
    mMat = nullptr;
    mMesh = nullptr;
    mLinePairs = 0;
    mHasCaps = 1;
    mFoldAngle = kDefaultFoldAngle;
    BuildMesh();
}

RndLine::~RndLine() {
    ReleaseMesh();
}

void RndLine::ListDrawObjects(std::list<RndObject *> &objects) {
    objects.push_back(mMesh->mMat);
}

void RndLine::ListDrawables(std::list<RndDrawable *> &drawables) {
    std::list<RndDrawable *> meshDrawables;
    mMesh->ListDrawables(meshDrawables);
    if (!meshDrawables.empty()) {
        drawables.push_back(this);
    }
    RndDrawable::ListDrawables(drawables);
}

void RndLine::SetHighlight(int nHighlight) {
    mMesh->SetHighlight(nHighlight);
}

int RndLine::DrawShowing() {
    RndCam *pCam = RndCam::sCurrent;
    if (pCam == nullptr) {
        return 1;
    }
    const int nPoints = static_cast<int>(mPoints.size());
    if (nPoints < 2) {
        return 1;
    }
    Transform xfm;
    Invert(xfm, pCam->mWorldXfm);
    Multiply(xfm, xfm, mWorldXfm);
    const float fNear = pCam->mNearPlane + kNearMargin;
    int nFirstBehind = -1;
    int nLastBehind = -1;
    for (int i = 0; i < nPoints; ++i) {
        Point &point = mPoints[i];
        MultiplyPoint(xfm, point.mPoint, point.mCamPoint);
        if (point.mCamPoint.y < fNear) {
            nLastBehind = i;
            if (nFirstBehind == -1) {
                nFirstBehind = i;
            }
        }
    }
    if (nFirstBehind == 0 && nLastBehind == nPoints - 1) {
        return 1;
    }
    if (mLinePairs == 0) {
        int nStart = 0;
        int nEnd = nPoints - 1;
        if (nLastBehind != -1) {
            const int nAfter = nLastBehind + 1;
            if (nPoints - nAfter < nFirstBehind) {
                Point &clipped = mPoints[nFirstBehind];
                const Point &visible = mPoints[nFirstBehind - 1];
                const float fRatio =
                    (fNear - visible.mCamPoint.y) / (clipped.mCamPoint.y - visible.mCamPoint.y);
                if (fRatio == 0.0f) {
                    clipped.mCamPoint = visible.mCamPoint;
                } else if (fRatio != 1.0f) {
                    ClipToNear(clipped.mCamPoint, visible.mCamPoint, 1.0f - fRatio);
                }
                nEnd = nFirstBehind;
            } else {
                Point &clipped = mPoints[nLastBehind];
                const Point &visible = mPoints[nAfter];
                const float fRatio =
                    (fNear - clipped.mCamPoint.y) / (visible.mCamPoint.y - clipped.mCamPoint.y);
                ClipToNear(clipped.mCamPoint, visible.mCamPoint, fRatio);
                nStart = nLastBehind;
            }
        }
        BuildStrip(&mPoints[nStart], &mPoints[nEnd]);
    } else {
        for (int i = 0; i < nPoints - 1; i += 2) {
            Point *pFirst = &mPoints[i];
            Point *pSecond = &mPoints[i + 1];
            if (pFirst->mCamPoint.y < fNear) {
                if (pSecond->mCamPoint.y < fNear) {
                    pSecond = pFirst;
                } else {
                    const float fRatio = (fNear - pFirst->mCamPoint.y) /
                                         (pSecond->mCamPoint.y - pFirst->mCamPoint.y);
                    ClipToNear(pFirst->mCamPoint, pSecond->mCamPoint, fRatio);
                }
            } else if (pSecond->mCamPoint.y < fNear) {
                const float fRatio =
                    (fNear - pSecond->mCamPoint.y) / (pFirst->mCamPoint.y - pSecond->mCamPoint.y);
                ClipToNear(pSecond->mCamPoint, pFirst->mCamPoint, fRatio);
            }
            BuildPair(pFirst, pSecond);
        }
    }
    mMesh->Sync(RndMesh::kSyncPositions);
    mMesh->mLocalXfm.mBasisX = pCam->mWorldXfm.mBasisX;
    mMesh->mLocalXfm.mBasisY = pCam->mWorldXfm.mBasisY;
    mMesh->mLocalXfm.mBasisZ = pCam->mWorldXfm.mBasisZ;
    mMesh->mDirty = 1;
    mMesh->mLocalXfm.mTranslation = pCam->mWorldXfm.mTranslation;
    mMesh->UpdateWorldXfm(nullptr, 0);
    mMesh->Draw();
    return 1;
}

void RndLine::Collide(const Segment &segment, std::list<Collision> &collisions) {
    if (mShowing == 0) {
        return;
    }
    const auto nBefore = collisions.size();
    mMesh->Collide(segment, collisions);
    auto it = collisions.begin();
    for (std::size_t i = 0; i < nBefore; ++i) {
        ++it;
    }
    for (; it != collisions.end(); ++it) {
        it->mObject = this;
    }
    RndCollideable::Collide(segment, collisions);
}

void RndLine::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndDrawable::DumpText(stream);
    RndCollideable::DumpText(stream);
    RndTransformable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndLine]\n";
    stream << "points:" << mPoints << "\n";
    stream << "width:" << mWidth << " foldAngle:" << mFoldAngle << " hasCaps:" << (mHasCaps != 0)
           << "\n";
    stream << "linePairs:" << (mLinePairs != 0) << "\n";
    if (stream.mDumpLevel < 2) {
        return;
    }
    stream << "mesh:" << static_cast<const RndObject *>(mMesh) << "\n";
}

void RndLine::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndDrawable::Save(stream);
    RndCollideable::Save(stream);
    RndTransformable::Save(stream);
    const RndMat *pMat = mMesh->mMat;
    stream.WriteString(pMat != nullptr ? pMat->mName.c_str() : "");
    stream << mPoints;
    stream.WriteEndian(&mWidth, sizeof(mWidth));
    stream.WriteEndian(&mFoldAngle, sizeof(mFoldAngle));
    WriteFlag(stream, mHasCaps);
    WriteFlag(stream, mLinePairs);
}

void RndLine::Replace(RndObject *pFrom, RndObject *pTo) {
    RndDrawable::Replace(pFrom, pTo);
    RndCollideable::Replace(pFrom, pTo);
    RndTransformable::Replace(pFrom, pTo);
}

void RndLine::Copy(const RndObject *pSource, int nFlags) {
    const RndLine *pLine = pSource != nullptr ? dynamic_cast<const RndLine *>(pSource) : nullptr;
    RndDrawable::Copy(pSource, nFlags);
    RndCollideable::Copy(pSource, nFlags);
    RndTransformable::Copy(pSource, nFlags);
    ReleaseMesh();
    mMat = pLine->mMesh->mMat;
    mPoints = pLine->mPoints;
    mWidth = pLine->mWidth;
    mFoldAngle = pLine->mFoldAngle;
    mHasCaps = pLine->mHasCaps;
    mLinePairs = pLine->mLinePairs;
    BuildMesh();
}

void RndLine::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Line");
        return;
    }
    RndDrawable::Load(stream);
    RndCollideable::Load(stream);
    RndTransformable::Load(stream);
    ReleaseMesh();
    mMat = FindByName<RndMat>(stream);
    stream >> mPoints;
    stream.ReadEndian(&mWidth, sizeof(mWidth));
    if (nRev >= kRevFold) {
        stream.ReadEndian(&mFoldAngle, sizeof(mFoldAngle));
        mHasCaps = ReadFlag(stream);
    }
    if (nRev >= kRevLinePairs) {
        mLinePairs = ReadFlag(stream);
    }
    BuildMesh();
}

RndMat *RndLine::Mat() const {
    return mMesh->mMat;
}

void RndLine::SetMat(RndMat *pMat) {
    mMesh->SetMat(pMat);
}

void RndLine::SetFoldAngle(float fAngle) {
    mFoldAngle = fAngle;
    mFoldCos = std::cos(fAngle);
}

void RndLine::SetNumPoints(int nPoints) {
    mPoints.resize(nPoints, Point());
    if (nPoints <= 0) {
        return;
    }
    int nVertPairs;
    if (mHasCaps == 0) {
        nVertPairs = nPoints;
    } else if (mLinePairs != 0) {
        nVertPairs = (nPoints >> 1) * 4;
    } else {
        nVertPairs = nPoints + 2;
    }
    const int nVerts = nVertPairs * 2;
    mMesh->mGeomOwner->mVerts.resize(nVerts, RndMesh::Vert());
    for (unsigned int i = 0; i < mPoints.size(); ++i) {
        const Color &color = mPoints[i].mColor;
        PointVerts verts;
        FindPointVerts(static_cast<int>(i), verts);
        RndMesh::Vert *pVert = verts.mVerts;
        if (verts.mCaps == 1) {
            pVert->mTex.y = 1.0f;
            pVert->mTex.x = 0.0f;
            pVert->mColor = color;
            ++pVert;
            pVert->mTex.y = 0.0f;
            pVert->mTex.x = 0.0f;
            pVert->mColor = color;
            ++pVert;
        }
        pVert->mTex.x = 0.5f;
        pVert->mTex.y = 1.0f;
        pVert->mColor = color;
        ++pVert;
        pVert->mTex.y = 0.0f;
        pVert->mTex.x = 0.5f;
        pVert->mColor = color;
        ++pVert;
        if (verts.mCaps == 2) {
            pVert->mTex.y = 1.0f;
            pVert->mTex.x = 1.0f;
            pVert->mColor = color;
            ++pVert;
            pVert->mTex.x = 1.0f;
            pVert->mTex.y = 0.0f;
            pVert->mColor = color;
        }
    }
    int nFaces;
    if (mLinePairs == 0) {
        nFaces = (nVertPairs - 1) * 2;
    } else if (mHasCaps != 0) {
        nFaces = (nVerts + nVertPairs) >> 1;
    } else {
        nFaces = nVertPairs;
    }
    std::vector<RndMesh::Face> &faces = mMesh->mGeomOwner->mFaces;
    faces.resize(nFaces, RndMesh::Face());
    for (int nFace = nFaces - 2; nFace >= 0; nFace -= 2) {
        int nVert;
        if (mLinePairs == 0) {
            nVert = nFace;
        } else if (mHasCaps == 0) {
            nVert = nFace * 2;
        } else {
            nVert = (nFace / 6) * 8 + nFace % 6;
        }
        RndMesh::Face &first = mMesh->mGeomOwner->mFaces[nFace];
        first.mVerts[2] = static_cast<unsigned short>(nVert + 1);
        first.mVerts[0] = static_cast<unsigned short>(nVert);
        first.mVerts[1] = static_cast<unsigned short>(nVert + 2);
        RndMesh::Face &second = mMesh->mGeomOwner->mFaces[nFace + 1];
        second.mVerts[2] = static_cast<unsigned short>(nVert + 3);
        second.mVerts[0] = static_cast<unsigned short>(nVert + 1);
        second.mVerts[1] = static_cast<unsigned short>(nVert + 2);
    }
    mMesh->Sync(kSyncPointLayout);
}

void RndLine::SetPoint(int nIndex, const Vector3 &point) {
    mPoints[nIndex].mPoint = point;
}

void RndLine::SetPointColor(int nIndex, const Color &color) {
    mPoints[nIndex].mColor = color;
    PointVerts verts;
    FindPointVerts(nIndex, verts);
    RndMesh::Vert *pVert = verts.mVerts;
    pVert[0].mColor = color;
    pVert[1].mColor = color;
    if (verts.mCaps != 0) {
        pVert[2].mColor = color;
        pVert[3].mColor = color;
    }
    mMesh->Sync(RndMesh::kSyncColors);
}

void RndLine::FindPointVerts(int nIndex, PointVerts &verts) {
    std::vector<RndMesh::Vert> &meshVerts = mMesh->mGeomOwner->mVerts;
    if (mHasCaps == 0) {
        verts.mCaps = 0;
        verts.mVerts = &meshVerts[nIndex * 2];
    } else if (mLinePairs != 0) {
        verts.mCaps = (nIndex & 1) != 0 ? 2 : 1;
        verts.mVerts = &meshVerts[nIndex * 4];
    } else if (nIndex == 0) {
        verts.mCaps = 1;
        verts.mVerts = &meshVerts[0];
    } else if (nIndex + 1 == static_cast<int>(mPoints.size())) {
        verts.mCaps = 2;
        verts.mVerts = &meshVerts[meshVerts.size() - 4];
    } else {
        verts.mCaps = 0;
        verts.mVerts = &meshVerts[(nIndex + 1) * 2];
    }
}

void RndLine::ReleaseMesh() {
    delete mMesh;
    mMesh = nullptr;
}

void RndLine::BuildMesh() {
    const char *pszMeshName = FormatString("[%s_mesh]", mName.c_str());
    RndObject *pObject = TheManager.Create(RndMesh::sClassName, pszMeshName);
    mMesh = pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr;
    mMesh->mInternal = 1;
    mMesh->mGeomOwner->mMutable = 1;
    mMesh->SetMat(mMat);
    mMesh->mZFunc = RndMesh::kZFuncLess;
    mMesh->mZMode = RndMesh::kZModeZReadOnly;
    mFoldCos = std::cos(mFoldAngle);
    SetNumPoints(static_cast<int>(mPoints.size()));
}

void RndLine::BuildStrip(Point *pFirst, Point *pLast) {
    for (Point *p = pFirst; p <= pLast; ++p) {
        Project(*p);
    }
    Point *p = pFirst;
    for (; p != pLast; ++p) {
        SetDirection(*p, p[1].mScreen, mWidth);
    }
    p->mDir = p[-1].mDir;
    p->mOffset = p[-1].mOffset;

    Rnd::Ray edge;
    edge.mPoint.x = pFirst->mScreen.x + pFirst->mOffset.x;
    edge.mPoint.y = pFirst->mScreen.y + pFirst->mOffset.y;
    edge.mDirection = pFirst->mDir;
    int bFlipped = 0;
    for (p = pFirst + 1; p != pLast; ++p) {
        const float fCos = p->mDir.x * p[-1].mDir.x + p->mDir.y * p[-1].mDir.y;
        if (fCos < mFoldCos) {
            bFlipped ^= 1;
        }
        if (bFlipped != 0) {
            p->mOffset.x = -p->mOffset.x;
            p->mOffset.y = -p->mOffset.y;
        }
        const Rnd::Ray previous = edge;
        edge.mPoint.x = p->mScreen.x + p->mOffset.x;
        edge.mPoint.y = p->mScreen.y + p->mOffset.y;
        edge.mDirection = p->mDir;
        if (fCos < kStraightCos) {
            p->mOffset = Rnd::Intersect(edge, previous);
            p->mOffset.x -= p->mScreen.x;
            p->mOffset.y -= p->mScreen.y;
        }
    }
    if (bFlipped != 0) {
        p->mOffset.x = -p->mOffset.x;
        p->mOffset.y = -p->mOffset.y;
    }

    Point *pBegin = &mPoints.front();
    if (pFirst == pBegin) {
        for (Point *pClipped = pLast + 1; pClipped <= &mPoints.back(); ++pClipped) {
            pClipped->mOffset = pLast->mOffset;
            pClipped->mCamPoint = pLast->mCamPoint;
        }
    } else {
        for (Point *pClipped = pBegin; pClipped < pFirst; ++pClipped) {
            pClipped->mOffset = pFirst->mOffset;
            pClipped->mCamPoint = pFirst->mCamPoint;
        }
    }

    Point *pEnd = &mPoints.back();
    Vector2 cap;
    cap.y = pBegin->mOffset.x;
    cap.x = -pBegin->mOffset.y;
    PointVerts verts;
    FindPointVerts(0, verts);
    RndMesh::Vert *pVert = verts.mVerts;
    if (mHasCaps != 0) {
        FillCap(*pBegin, cap, pVert);
    }
    for (p = pBegin; p <= pEnd; ++p) {
        FillPair(*p, pVert);
    }
    if (mHasCaps != 0) {
        if (bFlipped != 0) {
            cap.y = pEnd->mOffset.x;
            cap.x = -pEnd->mOffset.y;
        } else {
            cap.x = pEnd->mOffset.y;
            cap.y = -pEnd->mOffset.x;
        }
        FillCap(*pEnd, cap, pVert);
    }
}

void RndLine::BuildPair(Point *pFirst, Point *pSecond) {
    PointVerts verts;
    FindPointVerts(static_cast<int>(pFirst - &mPoints.front()), verts);
    RndMesh::Vert *pVert = verts.mVerts;
    if (pFirst == pSecond) {
        const int nVerts = mHasCaps != 0 ? 8 : 4;
        for (int i = 0; i < nVerts; ++i) {
            pVert[i].mPos = pSecond->mCamPoint;
        }
        return;
    }
    Project(*pFirst);
    Project(*pSecond);
    SetDirection(*pFirst, pSecond->mScreen, mWidth);
    pSecond->mOffset = pFirst->mOffset;
    Vector2 cap;
    cap.y = pFirst->mOffset.x;
    cap.x = -pFirst->mOffset.y;
    if (mHasCaps != 0) {
        FillCap(*pFirst, cap, pVert);
    }
    FillPair(*pFirst, pVert);
    FillPair(*pSecond, pVert);
    if (mHasCaps != 0) {
        cap.x = pSecond->mOffset.y;
        cap.y = -pSecond->mOffset.x;
        FillCap(*pSecond, cap, pVert);
    }
}

PrnStream &operator<<(PrnStream &stream, const RndLine::Point &point) {
    stream << "\n\tv:" << point.mPoint;
    stream << "\n\tc:" << point.mColor;
    return stream;
}

BinStream &operator<<(BinStream &stream, const RndLine::Point &point) {
    stream.WriteEndian(&point.mPoint.x, sizeof(point.mPoint.x));
    stream.WriteEndian(&point.mPoint.y, sizeof(point.mPoint.y));
    stream.WriteEndian(&point.mPoint.z, sizeof(point.mPoint.z));
    stream.WriteEndian(&point.mColor.r, sizeof(point.mColor.r));
    stream.WriteEndian(&point.mColor.g, sizeof(point.mColor.g));
    stream.WriteEndian(&point.mColor.b, sizeof(point.mColor.b));
    stream.WriteEndian(&point.mColor.a, sizeof(point.mColor.a));
    return stream;
}

BinStream &operator>>(BinStream &stream, RndLine::Point &point) {
    stream.ReadEndian(&point.mPoint.x, sizeof(point.mPoint.x));
    stream.ReadEndian(&point.mPoint.y, sizeof(point.mPoint.y));
    stream.ReadEndian(&point.mPoint.z, sizeof(point.mPoint.z));
    stream.ReadEndian(&point.mColor.r, sizeof(point.mColor.r));
    stream.ReadEndian(&point.mColor.g, sizeof(point.mColor.g));
    stream.ReadEndian(&point.mColor.b, sizeof(point.mColor.b));
    stream.ReadEndian(&point.mColor.a, sizeof(point.mColor.a));
    return stream;
}
