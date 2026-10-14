#include "gfx/tnlgrid.h"

#include <climits>
#include <cmath>

#include "game/gamedb.h"
#include "gfx/gfxtunnel.h"
#include "gfx/tnlgeom.h"
#include "rnd/rndmanager.h"

namespace {

// The place across a track of a line, the middle.
constexpr float kLineLateral = 0.5f;

// The height of a line, just below the surface of the track at 1.
constexpr float kLineRaise = 0.97f;

// The camera whose grid empties the shared multi-mesh first.
constexpr int kFirstCam = 1;

// The bar no line lies on, the first bar Update() compares.
constexpr int kNoBar = INT_MAX;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

Vector3 TnlGrid::sQuarterScale{1.0f, 1.0f, 1.0f};
Vector3 TnlGrid::sEighthScale{1.0f, 1.0f, 1.0f};

TnlGrid::GridLine::GridLine() : mScale(nullptr) {
}

TnlGrid::GridLine::~GridLine() = default;

void TnlGrid::GridLine::Attach(const Vector3 *pScale, RndMultiMesh *pMesh) {
    std::list<Transform> &instances = pMesh->mTransforms;
    Transform xfm; // Yes, the binary adds a transform it never set. Place() sets it.
    instances.insert(instances.begin(), xfm);
    mScale = pScale;
    mShowing = true;
    mInstance = instances.begin();
}

void TnlGrid::GridLine::Place(TnlGeom *pGeom, int nTrack, int nTick) {
    const float flTick = static_cast<float>(nTick);
    pGeom->PlaceCell(nTrack, &*mInstance, false, true, flTick, kLineLateral, kLineRaise);

    Transform &xfm = *mInstance;
    xfm.mBasisX.x *= mScale->x;
    xfm.mBasisX.y *= mScale->x;
    xfm.mBasisX.z *= mScale->x;
    xfm.mBasisY.x *= mScale->y;
    xfm.mBasisY.y *= mScale->y;
    xfm.mBasisY.z *= mScale->y;
    xfm.mBasisZ.x *= mScale->z;
    xfm.mBasisZ.y *= mScale->z;
    xfm.mBasisZ.z *= mScale->z;
    mTick = flTick;
}

void TnlGrid::GridLine::SetShowing(bool bShowing,
                                   RndMultiMesh *pMesh,
                                   std::list<Transform> &hidden) {
    if (bShowing == mShowing) {
        return;
    }

    std::list<Transform> &instances = pMesh->mTransforms;
    if (bShowing) {
        instances.splice(instances.begin(), hidden, mInstance);
        mInstance = instances.begin();
    } else {
        hidden.splice(hidden.end(), instances, mInstance);
        mInstance = hidden.end();
        --mInstance;
    }
    mShowing = bShowing;
}

TnlGrid::TnlGrid(GfxTunnel *pTunnel, int nCam) : mTrack(0), mTunnel(pTunnel), mMesh(nullptr) {
    mShowLines = true;
    mRefresh = true;
    if (nCam <= 0) {
        return;
    }

    mMesh = FindObject<RndMultiMesh>("grid.mm");
    if (nCam == kFirstCam) {
        mMesh->mTransforms.clear(); // Yes, the binary does not check for a missing mesh.
    }
    mLines.resize(kNumLines, GridLine());

    float flTick = 0.0f;
    bool bEighth = false;
    for (auto it = mLines.begin(); it != mLines.end(); ++it) {
        if (bEighth) {
            it->Attach(&sEighthScale, mMesh);
            bEighth = false;
        } else {
            it->Attach(&sQuarterScale, mMesh);
            bEighth = true;
        }
        it->Place(mTunnel->mGeom, mTrack, static_cast<int>(flTick));
        flTick += kLineTicks;
    }
}

TnlGrid::~TnlGrid() {
    if (mMesh != nullptr) {
        mMesh->mTransforms.clear();
    }
}

void TnlGrid::SetTrack(int nTrack) {
    const float flSongTick = TheGameDb->mSongTick;
    for (auto it = mLines.begin(); it != mLines.end(); ++it) {
        if (!(kRecycleTicks < flSongTick - it->mTick)) {
            it->Place(mTunnel->mGeom, nTrack, static_cast<int>(it->mTick));
        }
    }
    mTrack = nTrack;
    mRefresh = true;
}

void TnlGrid::SetOrder(int nOrder) {
    mShowLines = (nOrder == 0);
    mRefresh = true;
}

void TnlGrid::RefreshTrack(int nTrack, bool bPlace) {
    if (nTrack != mTrack) {
        return;
    }
    if (bPlace) {
        SetTrack(nTrack);
    } else {
        mRefresh = true;
    }
}

void TnlGrid::Update() {
    const float flSongTick = TheGameDb->mSongTick;
    int nLastBar = kNoBar;
    bool bBarShown = false;
    for (auto it = mLines.begin(); it != mLines.end(); ++it) {
        bool bMoved = mRefresh;
        if (kRecycleTicks < flSongTick - it->mTick) {
            it->Place(mTunnel->mGeom, mTrack, static_cast<int>(it->mTick + kSpanTicks));
            bMoved = true;
        } else if ((0.0f < flSongTick) && (flSongTick + kSpanTicks < it->mTick)) {
            float flSongPhase = fmodf(flSongTick, kSpanTicks);
            if (flSongPhase < 0.0f) {
                flSongPhase += kSpanTicks;
            }
            const float flSpanStart = flSongTick - flSongPhase;
            TnlGeom *pGeom = mTunnel->mGeom;
            float flLinePhase = fmodf(it->mTick, kSpanTicks);
            if (flLinePhase < 0.0f) {
                flLinePhase += kSpanTicks;
            }
            it->Place(pGeom, mTrack, static_cast<int>(flLinePhase + flSpanStart));
            bMoved = true;
        }

        if (!bMoved) {
            continue;
        }
        if (!mShowLines) {
            it->SetShowing(false, mMesh, mHidden);
            continue;
        }
        const int nBar = static_cast<int>(floorf(it->mTick * mTunnel->mGeom->mBarsPerTick));
        if (nBar != nLastBar) {
            nLastBar = nBar;
            bBarShown = mTunnel->HasActiveBar(mTrack, it->mTick);
        }
        it->SetShowing(bBarShown, mMesh, mHidden);
    }
    mRefresh = false;
}
