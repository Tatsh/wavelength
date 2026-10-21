#include "gfx/gfxutil.h"

#include <cmath>

#include "math/quaternion.h"
#include "math/transformops.h"
#include "os/debug.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtransanim.h"

namespace {

constexpr float kPi = 3.14159274f;
constexpr float kDegreesPerHalfTurn = 180.0f;

// The share of a track width the middle point of an even number of points sits left of centre.
constexpr float kHalf = 0.5f;

// The nodes of a `(value frame)` pair.
constexpr int kPairFrameNode = 0;
constexpr int kPairValueNode = 1;

// Find where a key of a frame goes among keys sorted by frame, after any key of the same frame
// except the last.
inline int KeyIndex(const std::vector<FloatKey> &keys, float fFrame) {
    if (keys.empty() || (fFrame <= keys.front().mFrame)) {
        return 0;
    }
    const int nSize = static_cast<int>(keys.size());
    if (keys.back().mFrame <= fFrame) {
        return (fFrame == keys.back().mFrame) ? (nSize - 1) : nSize;
    }
    int nLow = 0;
    int nHigh = nSize - 1;
    while ((nLow + 1) < nHigh) {
        const int nMiddle = (nLow + nHigh) >> 1;
        const float fMiddle = keys.at(nMiddle).mFrame;
        if (fFrame == fMiddle) {
            return nMiddle;
        }
        if (fMiddle < fFrame) {
            nLow = nMiddle;
        } else {
            nHigh = nMiddle;
        }
    }
    // Yes, the binary range-checks the keys on either side of the frame, and the lower one again,
    // and returns only the index.
    (void)keys.at(nLow);
    (void)keys.at(nHigh);
    (void)keys.at(nLow);
    return nHigh;
}

} // namespace

void DetachView(RndView *pParent, RndView *pChild) {
    pParent->RemoveDraw(pChild);
    pParent->RemoveAnim(pChild);
    pParent->RemoveTrans(pChild);
    pParent->RemoveCollide(pChild);
}

void AttachView(RndView *pParent, RndView *pChild) {
    pParent->AddDraw(pChild, nullptr);
    pParent->AddAnim(pChild);
    pParent->AddTrans(pChild);
}

void SetFilterScale(RndAnimatable *pAnim, float fScale) {
    RndAnimatable::ScaleOffset *pFilter = nullptr;
    if (!pAnim->GetFilters().empty()) {
        pFilter = dynamic_cast<RndAnimatable::ScaleOffset *>(pAnim->GetFilters().front());
    }
    if (pFilter == nullptr) {
        DebugWarn("%s must have scale-offset filter in slot 1", pAnim->GetName());
        return;
    }
    const float fFrame = pAnim->GetFrame();
    const float fFiltered = pFilter->Apply(fFrame);
    pFilter->SetScale(fScale);
    pFilter->SetOffset(fFiltered - (fFrame * fScale));
}

void Rotate(Vector2 *pPoint, float fAngle) {
    const float fCos = std::cos(fAngle);
    const float fSin = std::sin(fAngle);
    const float fX = pPoint->x;
    const float fY = pPoint->y;
    pPoint->y = (fSin * fX) + (fCos * fY);
    pPoint->x = (fCos * fX) - (fSin * fY);
}

void BuildTrackArc(std::vector<Vector2> *pPoints, DataArray *pConfig) {
    float fCenterHeight;
    float fTrackWidth;
    float fStartAngle;
    float fAngleInc;
    float fAngleIncInc;
    pConfig->FindFloat("center_height", &fCenterHeight, true);
    pConfig->FindFloat("track_width", &fTrackWidth, true);
    pConfig->FindFloat("start_angle", &fStartAngle, true);
    pConfig->FindFloat("angle_inc", &fAngleInc, true);
    pConfig->FindFloat("angle_inc_inc", &fAngleIncInc, true);
    fStartAngle = (fStartAngle * kPi) / kDegreesPerHalfTurn;
    fAngleInc = (fAngleInc * kPi) / kDegreesPerHalfTurn;
    fAngleIncInc = (fAngleIncInc * kPi) / kDegreesPerHalfTurn;

    std::vector<Vector2> &points = *pPoints;
    const int nPoints = static_cast<int>(points.size());
    Vector2 step{-fTrackWidth, 0.0f};
    const int nMiddle = (nPoints - 1) / 2;
    if ((nPoints % 2) == 0) {
        points[nMiddle].y = fCenterHeight;
        points[nMiddle].x = fTrackWidth * -kHalf;
    } else {
        points[nMiddle].y = fCenterHeight;
        points[nMiddle].x = 0.0f;
    }
    Rotate(&step, fStartAngle);
    float fAngle = fAngleInc;
    for (int i = nMiddle - 1; i >= 0; --i) {
        points[i].y = points[i + 1].y + step.y;
        points[i].x = points[i + 1].x + step.x;
        Rotate(&step, fAngle);
        fAngle += fAngleIncInc;
    }
    for (int i = nMiddle + 1; i < nPoints; ++i) {
        points[i] = points[nPoints - 1 - i];
        points[i].x = -points[i].x;
    }
}

void ResetXfmTree(RndTransformable *pRoot, bool bReset) {
    if (bReset) {
        pRoot->GetLocalXfm() = Transform{Vector3{1.0f, 0.0f, 0.0f},
                                         Vector3{0.0f, 1.0f, 0.0f},
                                         Vector3{0.0f, 0.0f, 1.0f},
                                         Vector3{0.0f, 0.0f, 0.0f, 1.0f}};
        pRoot->SetDirty(1);
    }
    for (RndTransformable *pChild : pRoot->GetTransList()) {
        ResetXfmTree(pChild, true);
    }
}

void SetZModeTree(RndTransformable *pRoot, RndMesh::ZMode zMode, RndMesh::ZFunc zFunc) {
    RndMesh *pMesh = (pRoot != nullptr) ? dynamic_cast<RndMesh *>(pRoot) : nullptr;
    if (pMesh != nullptr) {
        pMesh->SetZFunc(zFunc);
        pMesh->SetZMode(zMode);
    }
    for (RndTransformable *pChild : pRoot->GetTransList()) {
        SetZModeTree(pChild, zMode, zFunc);
    }
}

void ClearMeshSpheres(RndTransformable *pRoot) {
    RndMesh *pMesh = (pRoot != nullptr) ? dynamic_cast<RndMesh *>(pRoot) : nullptr;
    if (pMesh != nullptr) {
        pMesh->GetSphere().mCenter.x = 0.0f;
        pMesh->GetSphere().mCenter.y = 0.0f;
        pMesh->GetSphere().mCenter.z = 0.0f;
        pMesh->GetSphere().mRadius = 0.0f;
    }
    for (RndTransformable *pChild : pRoot->GetTransList()) {
        ClearMeshSpheres(pChild);
    }
}

void LoadFloatKeys(DataArray *pData, std::vector<FloatKey> *pKeys) {
    pKeys->clear();
    for (int i = 1; i < pData->Size(); ++i) {
        DataArray *pPair = pData->Array(i);
        const float fValue = pPair->Float(kPairValueNode);
        const float fFrame = pPair->Float(kPairFrameNode);
        const int nIndex = KeyIndex(*pKeys, fFrame);
        pKeys->insert(pKeys->begin() + nIndex, FloatKey{fValue, fFrame});
    }
}

void ScaleTransKeys(const char *pszName, float fScale) {
    RndTransAnim *pAnim = dynamic_cast<RndTransAnim *>(TheManager.Find(pszName));
    ScaleKeys(pAnim->GetFramesOwner()->GetTransKeys(), fScale);
}

void ScaleKeys(std::vector<Key<Vector3>> &keys, float fScale) {
    for (Key<Vector3> &key : keys) {
        key.value.x *= fScale;
        key.value.y *= fScale;
        key.value.z *= fScale;
    }
}

void InterpBasis(const float *pFrom, const float *pTo, float *pOut, float fBlend) {
    Quat from;
    from.Set(pFrom);
    Quat to;
    to.Set(pTo);
    QuatSlerp(from, to, from, fBlend);
    Rnd::MakeRotMatrix(from, pOut);
}

void ScaleParticles(RndParticleSys *pSys, float fScale) {
    pSys->GetSize().x *= fScale;
    pSys->GetSize().y *= fScale;
    Vector3 &force = pSys->GetForce();
    force.x *= fScale;
    force.y *= fScale;
    force.z *= fScale;
}

void ScaleParticleTree(RndTransformable *pRoot, float fScale) {
    if (pRoot == nullptr) {
        return;
    }
    RndParticleSys *pSys = dynamic_cast<RndParticleSys *>(pRoot);
    if (pSys != nullptr) {
        ScaleParticles(pSys, fScale);
    }
    for (RndTransformable *pChild : pRoot->GetTransList()) {
        ScaleParticleTree(pChild, fScale);
    }
}
