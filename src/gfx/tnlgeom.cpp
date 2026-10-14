#include "gfx/tnlgeom.h"

#include <algorithm>
#include <cmath>

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "math/transformops.h"
#include "math/vector3.h"
#include "os/debug.h"
#include "os/string.h"
#include "os/timer.h"
#include "rnd/manager.h"
#include "rnd/ps.h"

namespace {

// The pass of the standard panel that draws the lit colour of a solo game, and the pass that
// draws it in a game of several players.
constexpr int kSoloLitPass = 0;
constexpr int kMultiLitPass = 1;

// The animation of a pass drives the one stage of its material.
constexpr int kPassStageCount = 1;

template <typename T>
T *CreateObject(const char *pszClass, const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Create(pszClass, pszName));
}

// A move between two tracks takes this share of mMoveLength for each track crossed, on top of the
// base share.
constexpr float kMoveLengthPerTrack = 0.1f;
constexpr float kMoveLengthBase = 0.9f;

// Tracks a player crosses take InvExpInterpolator curves of this power.
constexpr float kMovePower = 2.0f;

// The samples of the table of a move of the camera and of a move of the player.
constexpr int kCamCurveSamples = 16;
constexpr int kMoveCurveSamples = 64;

// The X of the centre of a track in the space of a cross section.
constexpr float kTrackCenter = 0.5f;

// BuiltEndTick() stops this many ticks short of the end of the built bars.
constexpr float kBuiltEndMargin = 16.0f;

// The initial value of TnlGeom::Player::mTrailTicks, half a bar of ticks.
constexpr float kPlayerTrailTicks = -960.0f;

// The settings of a new geometry. A detail of 2 gives two cross sections a bar.
constexpr int kDefaultDetail = 2;
constexpr int kDefaultSectionsPerBar = 1 << (kDefaultDetail - 1);
constexpr float kDefaultSlabDepth = 0.2f;
constexpr float kDefaultSlabAngle = 0.0f;
constexpr float kDefaultMoveLength = 480.0f;
constexpr float kDefaultCamMoveSpeed = 0.01f;
constexpr float kBarTicks = 1920.0f;
constexpr int kDefaultPanelKinds = 4;
constexpr int kPlayerCapacity = 4;

// The indexes of mPassLightColors.
constexpr int kPanelPlain = 0;
constexpr int kPanelHighlighted = 1;

// The GS registers a pass of the standard panel sets, and the depth writes and the greater or
// equal depth test it selects.
constexpr int kGsRegTest1 = 0x47;
constexpr int kGsRegZbuf1 = 0x4e;
constexpr unsigned long long kZbufWriteDepth = 0;
constexpr unsigned long long kZbufZmskMask = 1ULL << 32;
constexpr unsigned long long kTestZtstGequal = 2ULL << 17;
constexpr unsigned long long kTestZtstMask = 3ULL << 17;

// The pass of a material a pass of the standard panel selects.
constexpr int kSelectPass = 0;

// The vertices of a row of a panel mesh, in the order of a row.
enum PanelVert {
    kVertTopLeft = 0,
    kVertTopRight = 1,
    kVertBottomLeft = 2,
    kVertBottomRight = 3,
};

// The top of a panel mesh runs between these X, and its slab is grey.
constexpr float kPanelLeftX = -0.5f;
constexpr float kPanelRightX = 0.5f;
constexpr float kSlabShade = 0.25f;

// The slab of a transparent panel mesh is this share of mSlabDepth deep.
constexpr float kShallowSlab = 0.25f;

// A source mesh is as wide as a track of this width when the geometry has no track.
constexpr float kDefaultTrackWidth = 1.0f;

// The level of detail of the faces of a source mesh.
constexpr int kSourceMeshLevel = 0;

// The flags the mesh of a new panel copies the opaque source mesh with.
constexpr unsigned kPanelCopyFlags = 0;

// Draw a mesh with a material, then take the material off it again.
inline void DrawWithMat(Rnd::Mesh *pMesh, Rnd::Mat *pMat) {
    pMesh->SetMat(pMat);
    static_cast<Rnd::Drawable *>(pMesh)->Draw();
    pMesh->SetMat(nullptr);
}

// The vertices of one row of a panel mesh.
constexpr int kRowVerts = 4;

// The parts of a panel mesh a rebuild reports as changed, the five low kSync bits.
constexpr int kRebuildSyncMask = 0x1f;

// Write the identity over the basis and the translation, leaving the padding of the basis rows.
inline void SetIdentity(Transform *pXfm) {
    pXfm->mBasisX.x = 1.0f;
    pXfm->mBasisX.y = 0.0f;
    pXfm->mBasisX.z = 0.0f;
    pXfm->mBasisY.x = 0.0f;
    pXfm->mBasisY.z = 0.0f;
    pXfm->mBasisY.y = 1.0f;
    pXfm->mBasisZ.x = 0.0f;
    pXfm->mBasisZ.z = 1.0f;
    pXfm->mBasisZ.y = 0.0f;
    pXfm->mTranslation = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
}

// Blend two rows, returning either row exactly at the ends.
inline Vector3 LerpRow(const Vector3 &from, const Vector3 &to, float flT) {
    if (flT == 0.0f) {
        return from;
    }
    if (flT == 1.0f) {
        return to;
    }
    const float flU = 1.0f - flT;
    return Vector3{(from.x * flU) + (to.x * flT),
                   (from.y * flU) + (to.y * flT),
                   (from.z * flU) + (to.z * flT),
                   to.w};
}

// Apply the basis and the translation of a transform to a point.
inline Vector3 XfmPoint(const Transform &xfm, const Vector3 &point) {
    return Vector3{(xfm.mBasisX.x * point.x) + (xfm.mBasisY.x * point.y) +
                       (xfm.mBasisZ.x * point.z) + xfm.mTranslation.x,
                   (xfm.mBasisX.y * point.x) + (xfm.mBasisY.y * point.y) +
                       (xfm.mBasisZ.y * point.z) + xfm.mTranslation.y,
                   (xfm.mBasisX.z * point.x) + (xfm.mBasisY.z * point.y) +
                       (xfm.mBasisZ.z * point.z) + xfm.mTranslation.z,
                   point.w};
}

// Apply three basis rows to a direction.
inline Vector3 RotateDir(const Vector3 *pBasis, const Vector3 &dir) {
    return Vector3{(pBasis[0].x * dir.x) + (pBasis[1].x * dir.y) + (pBasis[2].x * dir.z),
                   (pBasis[0].y * dir.x) + (pBasis[1].y * dir.y) + (pBasis[2].y * dir.z),
                   (pBasis[0].z * dir.x) + (pBasis[1].z * dir.y) + (pBasis[2].z * dir.z),
                   dir.w};
}

// Wrap a track into [0, nNumTracks).
inline float WrapTrack(float flTrack, float flNumTracks) {
    float flWrapped = std::fmod(flTrack, flNumTracks);
    if (flWrapped < 0.0f) {
        flWrapped += flNumTracks;
    }
    return flWrapped;
}

// Wrap a distance between tracks into [-nNumTracks / 2, nNumTracks / 2).
inline float WrapDistance(float flDistance, float flNumTracks) {
    const float flLow = -(flNumTracks * 0.5f);
    const float flSpan = (flNumTracks * 0.5f) - flLow;
    float flWrapped = std::fmod(flDistance - flLow, flSpan);
    if (flWrapped < 0.0f) {
        flWrapped += flSpan;
    }
    return flLow + flWrapped;
}

// Wrap a track index into [0, nNumTracks).
inline int WrapTrackIndex(int nTrack, int nNumTracks) {
    const int nWrapped = nTrack % nNumTracks;
    return (nWrapped < 0) ? (nWrapped + nNumTracks) : nWrapped;
}

// Fill the corners and the edges of a triangle a pick tests.
inline void SetTriangle(Rnd::TriangleTest *pTri,
                        const Vector3 &origin,
                        const Vector3 &first,
                        const Vector3 &second) {
    const float edge1[] = {first.x - origin.x, first.y - origin.y, first.z - origin.z};
    const float edge2[] = {second.x - origin.x, second.y - origin.y, second.z - origin.z};
    pTri->mVertex[0] = origin.x;
    pTri->mVertex[1] = origin.y;
    pTri->mVertex[2] = origin.z;
    pTri->mVertex[3] = origin.w;
    for (int i = 0; i < 3; ++i) {
        pTri->mEdge1[i] = edge1[i];
        pTri->mEdge2[i] = edge2[i];
    }
    CrossVec3(pTri->mEdge1, pTri->mEdge2, pTri->mNormal);
}

} // namespace

// The static initialiser of the unit, and the routine that runs it to construct the identity.
// NTSC-U/C: 0x001d34f0, PAL: 0x001dc290
// NTSC-U/C: 0x001d3590, PAL: 0x001dc330
Transform TnlGeom::sIdentity = {Vector3{1.0f, 0.0f, 0.0f},
                                Vector3{0.0f, 1.0f, 0.0f},
                                Vector3{0.0f, 0.0f, 1.0f},
                                Vector3{0.0f, 0.0f, 0.0f}};

bool TnlGeom::MeshHolder::sKeepMeshes = false;

void Rnd::Mesh::FillEdges(int nEdgeA,
                          int nEdgeB,
                          int nCount,
                          int nStride,
                          const Vector3 &pointA0,
                          const Vector3 &pointB0,
                          const Vector3 &pointA1,
                          const Vector3 &pointB1,
                          const Vector2 &texA0,
                          const Vector2 &texB0,
                          const Vector2 &texA1,
                          const Vector2 &texB1,
                          const Vector2 &tex2A0,
                          const Vector2 &tex2B0,
                          const Vector2 &tex2A1,
                          const Vector2 &tex2B1,
                          const Color &colorA0,
                          const Color &colorB0,
                          const Color &colorA1,
                          const Color &colorB1,
                          const Vector3 &normA,
                          const Vector3 &normB) {
    std::vector<MeshVert> &verts = mVertsOwner->mVerts;
    const unsigned nNeeded = static_cast<unsigned>(nCount * 2);
    if (verts.size() < nNeeded) {
        verts.resize(nNeeded,
                     MeshVert{Vector3{0.0f, 0.0f, 0.0f, 1.0f},
                              Vector3{0.0f, 0.0f, 0.0f, 1.0f},
                              Color{1.0f, 1.0f, 1.0f, 1.0f},
                              Vector2{0.0f, 0.0f},
                              Vector2{0.0f, 0.0f}});
    }
    const auto lerpPoint = [](const Vector3 &from, const Vector3 &to, float flT) {
        if (flT == 0.0f) {
            return from;
        }
        if (flT == 1.0f) {
            return to;
        }
        const float flU = 1.0f - flT;
        return Vector3{(to.x * flT) + (from.x * flU),
                       (to.y * flT) + (from.y * flU),
                       (to.z * flT) + (from.z * flU),
                       to.w};
    };
    const auto lerpTex = [](const Vector2 &from, const Vector2 &to, float flT) {
        return Vector2{((to.x - from.x) * flT) + from.x, ((to.y - from.y) * flT) + from.y};
    };
    const auto lerpColor = [](const Color &from, const Color &to, float flT) {
        const float flU = 1.0f - flT;
        return Color{(to.r * flT) + (from.r * flU),
                     (to.g * flT) + (from.g * flU),
                     (to.b * flT) + (from.b * flU),
                     (to.a * flT) + (from.a * flU)};
    };
    const float flStep = 1.0f / static_cast<float>(nCount - 1);
    for (int i = 0; i != nCount; ++i) {
        const float flT = static_cast<float>(i) * flStep;
        MeshVert &vertA = verts[nEdgeA];
        MeshVert &vertB = verts[nEdgeB];
        vertA.mPoint = lerpPoint(pointA0, pointA1, flT);
        vertB.mPoint = lerpPoint(pointB0, pointB1, flT);
        vertA.mTex1 = lerpTex(texA0, texA1, flT);
        vertB.mTex1 = lerpTex(texB0, texB1, flT);
        vertA.mColor = lerpColor(colorA0, colorA1, flT);
        vertB.mColor = lerpColor(colorB0, colorB1, flT);
        vertA.mTex2 = lerpTex(tex2A0, tex2A1, flT);
        vertB.mTex2 = lerpTex(tex2B0, tex2B1, flT);
        vertA.mNorm = normA;
        vertB.mNorm = normB;
        nEdgeA += nStride;
        nEdgeB += nStride;
    }
}

TnlGeom::TnlGeom()
    : mNumBars(0), mReserved04(kDefaultDetail), mSlabDepth(kDefaultSlabDepth),
      mSlabAngle(kDefaultSlabAngle), mPath(new TnlPath()), mBarsBehind(0),
      mMoveLength(kDefaultMoveLength), mCamMoveSpeed(kDefaultCamMoveSpeed), mWraps(0),
      mShowPanels(1), mAnimateMoves(0), mBarsPerTick(1.0f / kBarTicks), mTicksPerBar(kBarTicks),
      mSectionsPerBar(kDefaultSectionsPerBar), mFirstBar(0), mMinFirstBar(0), mEndBar(kNoBar),
      mBarValuesDirty(0), mPanelMats(kDefaultPanelKinds, nullptr), mNumTracks(0) {
    // Yes, the binary leaves mDefaultMat and mPassLightColors unset until Init().
    mPlayers.reserve(kPlayerCapacity);
}

TnlGeom::~TnlGeom() {
    Clear(true);
    delete mPath;
}

void TnlGeom::LoadConfig([[maybe_unused]] DataArray *pConfig,
                         [[maybe_unused]] DataArray *pDefaults,
                         [[maybe_unused]] int nOption) {
}

void TnlGeom::DrawPanels() {
    RebuildPanels();
    if (mShowPanels) {
        const int nFirstBar = (mFirstBar < 0) ? 0 : mFirstBar;
        const int nEndBar = (mFirstBar + mNumBars) - mBarsBehind;
        DrawCheckpoints(nFirstBar, nEndBar);
        DrawPass(mPassMats[0], nFirstBar, nEndBar, kPassColorBase);
        // Yes, the binary starts at the second pass without checking for a first.
        for (auto it = mPassMats.begin() + 1; it != mPassMats.end(); ++it) {
            DrawPass(*it, nFirstBar, nEndBar, kPassColorNone);
        }
        DrawBarValues(nFirstBar, nEndBar, false);
    }
    mAnimateMoves = 1;
}

void TnlGeom::DrawPanelsMultiplayer() {
    RebuildPanels();
    if (mShowPanels) {
        const int nFirstBar = (mFirstBar < 0) ? 0 : mFirstBar;
        const int nEndBar = (mFirstBar + mNumBars) - mBarsBehind;
        DrawCheckpoints(nFirstBar, nEndBar);
        DrawPass(mPassMats[0], nFirstBar, nEndBar, kPassColorNone);
        // Yes, the binary starts at the second pass without checking for a first.
        for (auto it = mPassMats.begin() + 1; it != mPassMats.end(); ++it) {
            DrawPass(*it, nFirstBar, nEndBar, kPassColorLit);
        }
        DrawBarValues(nFirstBar, nEndBar, false);
    }
    mAnimateMoves = 1;
}

void TnlGeom::DrawOverlays() {
    if (!mShowPanels || (mFirstBar >= 0)) {
        return;
    }
    const int nEndBar = mFirstBar + mNumBars;
    DrawBarValues(mFirstBar, (nEndBar < 0) ? nEndBar : 0, true);
}

void TnlGeom::DrawCheckpoints(int nFirstBar, int nEndBar) {
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        if (mTracks[nTrack].mOverlayMode != 0) {
            continue;
        }
        for (int nBar = nEndBar - 1; nBar >= nFirstBar; --nBar) {
            const int nSlot = nBar & kRingMask;
            if (mBarRing[nSlot] != nBar) {
                continue;
            }
            const PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            if (panel.mCheckpointMat != nullptr) {
                DrawWithMat(panel.mMesh, panel.mCheckpointMat);
            }
        }
    }
}

void TnlGeom::DrawPass(Rnd::Mat *pMat, int nFirstBar, int nEndBar, PassColor nColor) {
    ThePs.SetGsReg(kGsRegZbuf1, kZbufWriteDepth, kZbufZmskMask);
    ThePs.SetGsReg(kGsRegTest1, kTestZtstGequal, kTestZtstMask);
    (void)pMat->Select(kSelectPass); // Yes, the binary discards whether the pass was selected.
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        if (mTracks[nTrack].mOverlayMode != 0) {
            continue;
        }
        for (int nBar = nEndBar - 1; nBar >= nFirstBar; --nBar) {
            const int nSlot = nBar & kRingMask;
            if (mBarRing[nSlot] != nBar) {
                continue;
            }
            PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            if (panel.mTransparent || (panel.mCheckpointMat != nullptr)) {
                continue;
            }
            if (nColor == kPassColorLit) {
                pMat->SetLightColor(mPassLightColors[static_cast<int>(panel.mHighlighted)]);
            }
            if (nColor != kPassColorNone) {
                pMat->SetBaseColor(panel.mColor);
            }
            panel.mMesh->DrawFaces(sIdentity);
        }
    }
}

void TnlGeom::DrawBarValues(int nFirstBar, int nEndBar, bool bWriteDepth) {
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        const TrackData &track = mTracks[nTrack];
        const unsigned char nPlayers = track.mPlayers;
        Rnd::Mat *pOverlay = track.mOverlayMat;
        if ((nPlayers == 0) && (pOverlay == nullptr) && !mBarValuesDirty) {
            continue;
        }
        for (int nBar = nEndBar - 1; nBar >= nFirstBar; --nBar) {
            const int nSlot = nBar & kRingMask;
            if (mBarRing[nSlot] != nBar) {
                continue;
            }
            PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            Rnd::Mesh::ZMode savedZMode = Rnd::Mesh::kZModeDisable;
            Rnd::Mesh::ZFunc savedZFunc = Rnd::Mesh::kZFuncNever;
            if (bWriteDepth) {
                savedZMode = panel.mMesh->mZMode;
                panel.mMesh->mZMode = Rnd::Mesh::kZModeZReadWrite;
                savedZFunc = panel.mMesh->mZFunc;
            }
            if (!panel.mTransparent) {
                Rnd::Mat *pMat = mBarValues[nSlot];
                if (pMat == nullptr) {
                    pMat = pOverlay;
                }
                if (pMat != nullptr) {
                    DrawWithMat(panel.mMesh, pMat);
                }
            } else if ((nPlayers != 0) && (panel.mTransparent == 1)) {
                DrawWithMat(panel.mMesh, mBarMats[nSlot]);
            }
            if (bWriteDepth) {
                panel.mMesh->mZFunc = savedZFunc;
                panel.mMesh->mZMode = savedZMode;
            }
        }
    }
}

bool TnlGeom::Pick(const Rnd::Segment *pSegment, int *pnTrack, int *pnBar) {
    const int nEndBar = mFirstBar + mNumBars;
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        for (int nBar = nEndBar - 1; nBar >= mFirstBar; --nBar) {
            const int nSlot = nBar & kRingMask;
            if (mBarRing[nSlot] != nBar) {
                continue;
            }
            const PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            if (panel.mPickable && panel.Hit(pSegment)) {
                *pnTrack = nTrack;
                *pnBar = nBar;
                return true;
            }
        }
    }
    return false;
}

void TnlGeom::RebuildPanels() {
    const int nRows = mSectionsPerBar + 1;
    const int nEndBar = mFirstBar + mNumBars;
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        const bool bMoved = mTrackOffsetChanged[nTrack];
        const Vector3 *pOffset = &mTrackOffsets[nTrack];
        for (int nBar = mFirstBar; nBar < nEndBar; ++nBar) {
            const int nSlot = nBar & kRingMask;
            if (mBarRing[nSlot] != nBar) {
                continue;
            }
            PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            if (bMoved || panel.mDirty) {
                panel.Rebuild(&mMeshes[static_cast<int>(panel.mTransparent)], pOffset, nRows);
            }
        }
        mTrackOffsetChanged[nTrack] = false;
    }
}

void TnlGeom::Poll(float flTick, float flDeltaTicks) {
    float flTrail = 0.0f;
    for (const auto &player : mPlayers) {
        flTrail = std::min(flTrail, player.mTrailTicks);
    }
    mFirstBar = static_cast<int>(std::floor((flTick + flTrail) * mBarsPerTick));
    if (mFirstBar < mMinFirstBar) {
        mFirstBar = mMinFirstBar;
    }
    BuildBars();
    if (mPath->mSegments[1].mAnim == nullptr) {
        return;
    }
    for (auto &player : mPlayers) {
        player.Update(flTick, flDeltaTicks);
        Transform xfm;
        BlendCell(&xfm,
                  true,
                  false,
                  player.mCamPosition,
                  flTick + player.mActivatorPosition,
                  kTrackCenter);
        player.PlaceActivator(&xfm);
    }
    for (Rnd::MatAnim *pAnim : mPassAnims) {
        pAnim->SetFrame(flTick);
    }
}

void TnlGeom::BuildRing() {
    Clear(false); // Yes, the binary asks for nothing to be cleared.
    mTicksPerBar = kBarTicks;
    mNumTracks = static_cast<int>(mTrackGeometry.size());
    mBarsPerTick = static_cast<float>(1.0 / static_cast<double>(kBarTicks));

    mBarRing.resize(mNumBars, 0);
    std::fill(mBarRing.begin(), mBarRing.end(), kNoBar);
    mBarMats.resize(mNumBars, nullptr);
    std::fill(mBarMats.begin(), mBarMats.end(), mDefaultMat);
    mBarValues.resize(mNumBars, nullptr);
    std::fill(mBarValues.begin(), mBarValues.end(), nullptr);
    mTrackOffsets.resize(mNumTracks);
    mTrackOffsetChanged.resize(mNumTracks, false);
    std::fill(mTrackOffsets.begin(), mTrackOffsets.end(), Vector3{0.0f, 0.0f, 0.0f});
    std::fill(mTrackOffsetChanged.begin(), mTrackOffsetChanged.end(), true);
    mTrackGeometry.resize(mNumTracks); // Yes, the size is already mNumTracks.

    BuildTrackGeometry();
    BuildPanels();
}

void TnlGeom::BuildTrackGeometry() {
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        TrackGeometry &geometry = mTrackGeometry[nTrack];
        Vector3 &across = geometry.mBasis[0];
        across.x = geometry.mRightX - geometry.mLeftX;
        across.y = 0.0f;
        across.z = geometry.mRightZ - geometry.mLeftZ;
        const float flInvLength =
            1.0f / std::sqrt((across.x * across.x) + (across.y * across.y) + (across.z * across.z));
        across.x *= flInvLength;
        across.y *= flInvLength;
        across.z *= flInvLength;

        Vector3 &up = geometry.mBasis[1];
        up.x = 0.0f;
        up.y = 1.0f;
        up.z = 0.0f;
        CrossVec3(&across.x, &up.x, &geometry.mBasis[2].x);
        geometry.mBasis[2].w = across.w;

        geometry.mCenter.x =
            ((geometry.mRightX - geometry.mLeftX) * kTrackCenter) + geometry.mLeftX;
        geometry.mCenter.z =
            ((geometry.mRightZ - geometry.mLeftZ) * kTrackCenter) + geometry.mLeftZ;
        geometry.mCenter.y = 0.0f;
        geometry.mRotation.Set(&geometry.mBasis[0].x);

        const float flDeltaX = geometry.mRightX - geometry.mLeftX;
        const float flDeltaZ = geometry.mRightZ - geometry.mLeftZ;
        const float flWidth = std::sqrt((flDeltaX * flDeltaX) + (flDeltaZ * flDeltaZ));
        const float aflScale[] = {flWidth, 1.0f, 1.0f};
        Vector3 *const apRows[] = {
            &geometry.mXfm.mBasisX, &geometry.mXfm.mBasisY, &geometry.mXfm.mBasisZ};
        for (int i = 0; i < 3; ++i) {
            apRows[i]->x = geometry.mBasis[i].x * aflScale[i];
            apRows[i]->y = geometry.mBasis[i].y * aflScale[i];
            apRows[i]->z = geometry.mBasis[i].z * aflScale[i];
        }
        geometry.mXfm.mTranslation = geometry.mCenter;
    }
}

void TnlGeom::Clear(bool bClear) {
    if (!bClear) {
        return;
    }
    mPanels.clear();
    mTracks.clear();
    for (unsigned i = 0; i < mPassMats.size(); ++i) {
        delete mPassMats[i];
        mPassMats[i] = nullptr;
    }
    for (unsigned i = 0; i < mPassAnims.size(); ++i) {
        delete mPassAnims[i];
        mPassAnims[i] = nullptr;
    }
}

void TnlGeom::BuildPanels() {
    {
        const PanelData blank;
        mPanels.resize(mNumTracks * mNumBars, blank);
    }
    const unsigned nSections = static_cast<unsigned>(mSectionsPerBar + 1);
    int nIndex = 0;
    for (auto &panel : mPanels) {
        if (panel.mMesh == nullptr) {
            panel.mMesh = CreateObject<Rnd::Mesh>(Rnd::Mesh::sClassName.mStr,
                                                  FormatString("panel %d dest.mesh", nIndex));
            panel.mMesh->mVertsOwner->mMutable = 1;
            panel.mMesh->Copy(mMeshes[kSourceMeshOpaque].mMesh, kPanelCopyFlags);
            panel.mDirty = 1;
        }
        ++nIndex;
        panel.mBaseSections.resize(nSections);
        panel.mSections.resize(nSections);
    }
}

Rnd::Mesh *TnlGeom::BuildPanelMesh(float flWidth, int nLevel, Vector3 *pExtent, bool bSolid) {
    const int nRows = mSectionsPerBar + 1;
    const float flSections = static_cast<float>(mSectionsPerBar);
    const Color white{1.0f, 1.0f, 1.0f, 1.0f};
    const Color slab{kSlabShade, kSlabShade, kSlabShade, bSolid ? 1.0f : 0.0f};

    Rnd::Mesh *pMesh = CreateObject<Rnd::Mesh>(Rnd::Mesh::sClassName.mStr, "tunnel drawdummy");
    pMesh->mVertsOwner->mMutable = 1;
    pMesh->mZMode = Rnd::Mesh::kZModeZReadWrite;
    pMesh->mZFunc = Rnd::Mesh::kZFuncLessEqual;
    pMesh->mVertsOwner->mVerts.resize(nRows * kRowVerts,
                                      Rnd::MeshVert{Vector3{0.0f, 0.0f, 0.0f, 1.0f},
                                                    Vector3{0.0f, 0.0f, 0.0f, 1.0f},
                                                    white,
                                                    Vector2{0.0f, 0.0f},
                                                    Vector2{0.0f, 0.0f}});
    pExtent->y = flSections;
    pExtent->z = 0.0f;
    pExtent->x = 1.0f;

    const float flInvWidth = 1.0f / flWidth;
    const Vector3 topLeft{kPanelLeftX, 0.0f, 0.0f};
    const Vector3 topRight{kPanelRightX, 0.0f, 0.0f};
    const Vector3 up{0.0f, 0.0f, 1.0f};
    const Vector2 texFirstLeft{0.0f, 1.0f};
    const Vector2 texFirstRight{1.0f, 1.0f};
    const Vector2 texLastLeft{0.0f, 0.0f};
    const Vector2 texLastRight{1.0f, 0.0f};
    const Vector2 tex2First{0.0f, 0.0f};
    const Vector2 tex2Last{1.0f, 0.0f};
    pMesh->FillEdges(kVertTopLeft,
                     kVertTopRight,
                     nRows,
                     kRowVerts,
                     topLeft,
                     topRight,
                     topLeft,
                     topRight,
                     texFirstLeft,
                     texFirstRight,
                     texLastLeft,
                     texLastRight,
                     tex2First,
                     tex2First,
                     tex2Last,
                     tex2Last,
                     white,
                     white,
                     white,
                     white,
                     up,
                     up);

    const float flCos = std::cos(mSlabAngle);
    const float flSin = std::sin(mSlabAngle);
    const float flDepth = bSolid ? mSlabDepth : (mSlabDepth * kShallowSlab);
    const float flInset = (flSin * flDepth) * flInvWidth;
    const Vector3 bottomLeft{kPanelLeftX + flInset, 0.0f, -(flCos * flDepth)};
    const Vector3 bottomRight{kPanelRightX - flInset, 0.0f, -(flCos * flDepth)};
    const Vector3 leftNorm{-flCos, 0.0f, -flSin};
    const Vector3 rightNorm{flCos, 0.0f, -flSin};
    pMesh->FillEdges(kVertBottomLeft,
                     kVertBottomRight,
                     nRows,
                     kRowVerts,
                     bottomLeft,
                     bottomRight,
                     bottomLeft,
                     bottomRight,
                     texFirstLeft,
                     texFirstRight,
                     texLastLeft,
                     texLastRight,
                     tex2First,
                     tex2First,
                     tex2Last,
                     tex2Last,
                     slab,
                     slab,
                     slab,
                     slab,
                     leftNorm,
                     rightNorm);

    const int nStep = 1 << nLevel;
    pMesh->AddQuadStrip(kVertTopRight, kVertTopLeft, nRows, kRowVerts, nStep);
    pMesh->AddQuadStrip(kVertBottomLeft, kVertBottomRight, nRows, kRowVerts, nStep);
    pMesh->AddQuadStrip(kVertTopLeft, kVertBottomLeft, nRows, kRowVerts, nStep);
    pMesh->AddQuadStrip(kVertBottomRight, kVertTopRight, nRows, kRowVerts, nStep);
    pMesh->AddQuad(kVertTopLeft, kVertBottomLeft, kVertTopRight, kVertBottomRight);
    return pMesh;
}

void TnlGeom::BuildMeshes() {
    mMeshes.clear();
    mMeshes.resize(kSourceMeshCount, MeshHolder{nullptr});
    BuildSourceMeshes();
}

void TnlGeom::BuildSourceMeshes() {
    float flWidth = kDefaultTrackWidth;
    if (!mTrackGeometry.empty()) {
        flWidth = mTrackGeometry[0].mRightX - mTrackGeometry[0].mLeftX;
    }
    int nIndex = 0;
    for (auto &holder : mMeshes) {
        delete holder.mMesh;
        holder.mMesh = nullptr;
        Vector3 extent;
        if (nIndex == kSourceMeshTransparent) {
            holder.mMesh = BuildPanelMesh(flWidth, kSourceMeshLevel, &extent, false);
            holder.mMesh->mZFunc = Rnd::Mesh::kZFuncLessEqual;
            holder.mMesh->mZMode = Rnd::Mesh::kZModeZReadOnly;
        } else {
            holder.mMesh = BuildPanelMesh(flWidth, kSourceMeshLevel, &extent, true);
        }
        holder.mMesh->SetName(FormatString("panel fx %d src.mesh", nIndex));
        ++nIndex;
    }
}

bool TnlGeom::Init(const std::vector<std::pair<Vector2, Vector2>> *pEdges,
                   int nNumBars,
                   int nDetail,
                   int nBarsBehind,
                   float flSlabDepth,
                   float flSlabAngle,
                   Rnd::TransAnim *pPath,
                   const std::vector<Color> *pColors,
                   Rnd::Mat *pPanelMat,
                   Rnd::MatAnim *pPanelAnim) {
    if (pEdges->empty()) {
        DebugNotify("Tunnel geometry has no tracks");
        return false;
    }
    mPath->Set(pPath, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    mNumBars = nNumBars;
    mBarsBehind = nBarsBehind;
    mSectionsPerBar = 1 << (nDetail - 1);
    mSlabDepth = flSlabDepth;
    mSlabAngle = flSlabAngle;
    mReserved04 = nDetail;

    mTrackGeometry.resize(pEdges->size());
    mTracks.resize(pEdges->size());
    for (unsigned i = 0; i < mTrackGeometry.size(); ++i) {
        TrackGeometry &geometry = mTrackGeometry[i];
        const std::pair<Vector2, Vector2> &edge = (*pEdges)[i];
        geometry.mLeftX = edge.first.x;
        geometry.mLeftZ = edge.first.y;
        geometry.mRightX = edge.second.x;
        geometry.mRightZ = edge.second.y;
        const int nLastColor = static_cast<int>(pColors->size()) - 1;
        mTracks[i].mColor = (*pColors)[std::min(static_cast<int>(i), nLastColor)];
        mTracks[i].mPlayers = 0;
        mTracks[i].mOverlayMat = nullptr;
    }

    for (unsigned i = 0; i < mPassMats.size(); ++i) {
        delete mPassMats[i];
        mPassMats[i] = nullptr;
    }
    for (unsigned i = 0; i < mPassAnims.size(); ++i) {
        delete mPassAnims[i];
        mPassAnims[i] = nullptr;
    }

    const int nPasses = static_cast<int>(pPanelMat->mStages.size());
    mPassMats.resize(nPasses, nullptr);
    const bool bMulti = !(TheGameDb->mCommunity == GameDb::kCommunitySolo ||
                          TheGameDb->mRuleSet != GameDb::kRuleSetGame);
    mPassLightColors[kPanelPlain] = Color{0.0f, 0.0f, 0.0f, 1.0f};
    mPassLightColors[kPanelHighlighted] = pPanelMat->mDiffuse;
    for (int i = 0; i < nPasses; ++i) {
        Rnd::Mat *pMat = CreateObject<Rnd::Mat>(Rnd::Mat::sClassName.mStr,
                                                FormatString("tnl std panel mat pass %d", i));
        mPassMats[i] = pMat;
        pMat->AddStage();
        pMat->SetAlpha(1.0f);
        pMat->SetBaseColor(Color{0.0f, 0.0f, 0.0f, 0.0f});
        pMat->SetLightColor(Color{0.0f, 0.0f, 0.0f, 0.0f});
        pMat->SetEdgeColor(Color{0.0f, 0.0f, 0.0f, 0.0f});
        Rnd::Mat::Stage &stage = pMat->mStages[0];
        const Rnd::Mat::Stage &source = pPanelMat->mStages[i];
        // The binary passes six lighting flags. The seventh stays clear.
        if (!bMulti && i == kSoloLitPass) {
            pMat->mBlend = pPanelMat->mBlend;
            stage.mBlend = source.mBlend;
            pMat->SetLighting(1, 0, 0, 0, 0, 0, 0);
        } else if (bMulti && i == kMultiLitPass) {
            pMat->mBlend = source.mBlend;
            stage.mBlend = Rnd::Mat::kBlendModeMultiply;
            pMat->SetLighting(1, 0, 0, 0, 0, 0, 0);
        } else {
            pMat->mBlend = (i != 0) ? source.mBlend : pPanelMat->mBlend;
            stage.mBlend = Rnd::Mat::kBlendModeSrc;
            pMat->SetLighting(0, 0, 0, 0, 0, 0, 0);
        }
        if (TheGfxManager.mNoPanels != 0 && i == 0) {
            pMat->mBlend = Rnd::Mat::kBlendModeDest;
        }
        pMat->mCull = pPanelMat->mCull;
        stage.mXfm = source.mXfm;
        stage.mUseXfm = source.mUseXfm;
        stage.mWrap = source.mWrap;
        stage.SetTex(source.mTex);
    }

    const auto hasKeys = [](const Rnd::MatAnim::Stage &stage) {
        return !stage.mTranslateKeys.empty() || !stage.mScaleKeys.empty() ||
               !stage.mRotateKeys.empty();
    };
    int nAnims = 0;
    for (unsigned i = 0; i < pPanelAnim->mKeysOwner->mStages.size(); ++i) {
        if (hasKeys(pPanelAnim->mKeysOwner->mStages[i])) {
            ++nAnims;
        }
    }
    mPassAnims.resize(nAnims, nullptr);
    nAnims = 0;
    for (unsigned i = 0; i < pPanelAnim->mKeysOwner->mStages.size(); ++i) {
        const Rnd::MatAnim::Stage &source = pPanelAnim->mKeysOwner->mStages[i];
        if (!hasKeys(source)) {
            continue;
        }
        Rnd::MatAnim *pAnim = CreateObject<Rnd::MatAnim>(
            Rnd::MatAnim::sClassName.mStr, FormatString("tnl std matanim stage %d", i));
        for (auto it = pPanelAnim->mFilters.begin(); it != pPanelAnim->mFilters.end(); ++it) {
            Rnd::Animatable::Filter *pFilter = Rnd::Animatable::NewFilter((*it)->Type());
            pFilter->Copy(*it);
            pAnim->mFilters.push_back(pFilter);
        }
        pAnim->SetMat(mPassMats[i]);
        pAnim->SetNumStages(kPassStageCount);
        mPassAnims[nAnims++] = pAnim;
        Rnd::MatAnim::Stage &target = pAnim->mKeysOwner->mStages[0];
        if (!source.mTranslateKeys.empty()) {
            target.mTranslateKeys = source.mTranslateKeys;
        }
        if (!source.mRotateKeys.empty()) {
            target.mRotateKeys = source.mRotateKeys;
        }
        if (!source.mScaleKeys.empty()) {
            target.mScaleKeys = source.mScaleKeys;
        }
    }

    BuildMeshes();
    BuildRing();
    return true;
}

void TnlGeom::BuildBars() {
    for (int nBar = mFirstBar; nBar < mFirstBar + mNumBars; ++nBar) {
        const int nSlot = WrapTrackIndex(nBar, mNumBars);
        if (nBar >= mEndBar) {
            mBarRing[nSlot] = kNoBar;
        } else if (mBarRing[nSlot] != nBar) {
            BuildBar(nBar);
            if (mAnimateMoves) {
                return;
            }
        }
    }
}

void TnlGeom::BuildBar(int nBar) {
    const int nSlot = WrapTrackIndex(nBar, mNumBars);
    float flTick = static_cast<float>(nBar) * mTicksPerBar;
    const float flStep = mTicksPerBar / static_cast<float>(mSectionsPerBar);
    for (int nSection = 0;; ++nSection) {
        bool bLast = false;
        if (nSection == mSectionsPerBar) {
            flTick = static_cast<float>(nBar + 1) * mTicksPerBar;
            bLast = true;
        }
        Transform path;
        const float flTime = flTick * *TheGfxManager.mMsPerTick;
        if (mPath->mSegments[1].mAnim == nullptr) {
            SetIdentity(&path);
        } else {
            mPath->Xfm(flTick, flTime, &path);
        }
        for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
            PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            CrossSectionXfm &section = panel.mBaseSections[nSection];
            const TrackGeometry &geometry = mTrackGeometry[nTrack];
            sceVu0MulAffineMatrixXyz(
                &section.mXfm.mBasisX.x, &path.mBasisX.x, &geometry.mXfm.mBasisX.x);
            for (int i = 0; i < 3; ++i) {
                section.mCellBasis[i] = RotateDir(&path.mBasisX, geometry.mBasis[i]);
            }
            panel.mSections[nSection] = section;
            if (bLast) {
                panel.mTransparent = 1;
                panel.mMesh->mZFunc = Rnd::Mesh::kZFuncLessEqual;
                panel.mMesh->mZMode = Rnd::Mesh::kZModeZReadOnly;
                panel.mCheckpointMat = nullptr;
                panel.mDirty = 1;
                panel.mPickable = 0;
                panel.mColor = mTracks[nTrack].mColor;
                panel.mHighlighted = 0;
            }
        }
        if (bLast) {
            break;
        }
        flTick += flStep;
    }

    mBarRing[nSlot] = nBar;
    const int nKind = nBar + static_cast<int>(mPanelMats.size());
    if ((nBar < 0) && (nKind >= 0)) {
        mBarMats[nSlot] = mPanelMats[nKind];
    } else {
        mBarMats[nSlot] = mDefaultMat;
    }
    mBarValues[nSlot] = nullptr;
}

void TnlGeom::ResetPanels() {
    mAnimateMoves = 0;
    for (auto &panel : mPanels) {
        panel.mDirty = 1;
        panel.mTransparent = 1;
        panel.mMesh->mZFunc = Rnd::Mesh::kZFuncLessEqual;
        panel.mMesh->mZMode = Rnd::Mesh::kZModeZReadOnly;
        panel.mCheckpointMat = nullptr;
        panel.mPickable = 0;
        panel.mColor = Color{1.0f, 1.0f, 1.0f, 1.0f};
        panel.mHighlighted = 0;
    }
    std::fill(mBarRing.begin(), mBarRing.end(), kNoBar);
}

void TnlGeom::ResetPlayers() {
    for (auto &player : mPlayers) {
        player.Reset();
    }
}

void TnlGeom::SetNumPlayers(int nPlayers) {
    mPlayers.resize(nPlayers);
    for (unsigned i = 0; i < mPlayers.size(); ++i) {
        mPlayers[i].Init(this, static_cast<char>(i));
    }
}

TnlGeom::Player *TnlGeom::GetPlayer(int nPlayer) {
    if (static_cast<unsigned>(nPlayer) >= mPlayers.size()) {
        return nullptr;
    }
    return &mPlayers[nPlayer];
}

void TnlGeom::SetMoveLength(float flLength) {
    mMoveLength = flLength;
}

float TnlGeom::MoveLength(float flFrom, float flTo) const {
    return ((std::fabs(flTo - flFrom) * kMoveLengthPerTrack) + kMoveLengthBase) * mMoveLength;
}

void TnlGeom::TrackXfm(
    int nTrack, Transform *pXfm, bool bSmoothBasis, float flTick, float flLateral) {
    if (mPath->mSegments[1].mAnim == nullptr) {
        SetIdentity(pXfm);
        return;
    }
    const float flBars = flTick * mBarsPerTick;
    const float flFloor = std::floor(flBars);
    const int nBar = static_cast<int>(flFloor);
    const int nSlot = nBar & kRingMask;
    if (mBarRing[nSlot] == nBar) {
        const float flSections = (flBars - flFloor) * static_cast<float>(mSectionsPerBar);
        int nSection = static_cast<int>(flSections);
        float flBlend;
        if (mSectionsPerBar < (nSection + 1)) {
            flBlend = 1.0f;
            --nSection;
        } else {
            flBlend = flSections - static_cast<float>(nSection);
        }
        const PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
        const CrossSectionXfm &first = panel.mBaseSections[nSection];
        const CrossSectionXfm &second = panel.mBaseSections[nSection + 1];
        if (bSmoothBasis) {
            pXfm->mBasisY = LerpRow(first.mCellBasis[1], second.mCellBasis[1], flBlend);
            pXfm->mBasisZ = LerpRow(first.mCellBasis[2], second.mCellBasis[2], flBlend);
            Mat33BuildOrthonormal(&pXfm->mBasisY.x, &pXfm->mBasisZ.x, &pXfm->mBasisX.x);
        } else {
            pXfm->mBasisX = first.mCellBasis[0];
            pXfm->mBasisY = first.mCellBasis[1];
            pXfm->mBasisZ = first.mCellBasis[2];
        }
        const Vector3 point{flLateral - kTrackCenter, 0.0f, 0.0f};
        pXfm->mTranslation =
            LerpRow(XfmPoint(first.mXfm, point), XfmPoint(second.mXfm, point), flBlend);
        return;
    }

    // Yes, the timer is started and never read.
    Timer timer;
    timer.Start();
    const TrackGeometry &geometry = mTrackGeometry[nTrack];
    pXfm->mBasisX = geometry.mBasis[0];
    pXfm->mBasisY = geometry.mBasis[1];
    pXfm->mBasisZ = geometry.mBasis[2];
    pXfm->mTranslation.x = ((geometry.mRightX - geometry.mLeftX) * flLateral) + geometry.mLeftX;
    pXfm->mTranslation.z = ((geometry.mRightZ - geometry.mLeftZ) * flLateral) + geometry.mLeftZ;
    pXfm->mTranslation.y = 0.0f;
    Transform path;
    mPath->Xfm(flTick, flTick * *TheGfxManager.mMsPerTick, &path);
    sceVu0MulAffineMatrixXyz(&pXfm->mBasisX.x, &path.mBasisX.x, &pXfm->mBasisX.x);
}

void TnlGeom::BlendTrack(
    Transform *pXfm, bool bSmoothBasis, float flTrack, float flTick, float flLateral) {
    if (mPath->mSegments[1].mAnim == nullptr) {
        SetIdentity(pXfm);
        return;
    }
    const float flFloor = std::floor(flTrack);
    const int nTrack = static_cast<int>(flFloor);
    const float flBlend = flTrack - flFloor;
    const int nFirst = WrapTrackIndex(nTrack, mNumTracks);
    const int nSecond = WrapTrackIndex(nTrack + 1, mNumTracks);
    Transform first;
    Transform second;
    TrackXfm(nFirst, &first, bSmoothBasis, flTick, flLateral);
    TrackXfm(nSecond, &second, bSmoothBasis, flTick, flLateral);
    pXfm->mTranslation = LerpRow(first.mTranslation, second.mTranslation, flBlend);
    pXfm->mBasisZ = LerpRow(first.mBasisZ, second.mBasisZ, flBlend);
    // Yes, the Y axis comes from the first track alone.
    Mat33BuildOrthonormal(&first.mBasisY.x, &pXfm->mBasisZ.x, &pXfm->mBasisX.x);
}

float TnlGeom::BuiltEndTick() const {
    return (static_cast<float>(mFirstBar + mNumBars) * mTicksPerBar) - kBuiltEndMargin;
}

void TnlGeom::CellXfm(int nTrack,
                      Transform *pXfm,
                      bool bSmoothBasis,
                      bool bTrackOffset,
                      float flTick,
                      float flLateral) {
    if (mPath->mSegments[1].mAnim == nullptr) {
        SetIdentity(pXfm);
        return;
    }
    const float flBars = flTick * mBarsPerTick;
    const float flFloor = std::floor(flBars);
    const int nBar = static_cast<int>(flFloor);
    const int nSlot = nBar & kRingMask;
    if (mBarRing[nSlot] == nBar) {
        const float flSections = (flBars - flFloor) * static_cast<float>(mSectionsPerBar);
        int nSection = static_cast<int>(flSections);
        float flBlend;
        if (mSectionsPerBar < (nSection + 1)) {
            flBlend = 1.0f;
            --nSection;
        } else {
            flBlend = flSections - static_cast<float>(nSection);
        }
        const PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
        const CrossSectionXfm &first = panel.mSections[nSection];
        const CrossSectionXfm &second = panel.mSections[nSection + 1];
        if (bSmoothBasis) {
            pXfm->mBasisY = LerpRow(first.mCellBasis[1], second.mCellBasis[1], flBlend);
            pXfm->mBasisZ = LerpRow(first.mCellBasis[2], second.mCellBasis[2], flBlend);
            Mat33BuildOrthonormal(&pXfm->mBasisY.x, &pXfm->mBasisZ.x, &pXfm->mBasisX.x);
        } else {
            pXfm->mBasisX = first.mCellBasis[0];
            pXfm->mBasisY = first.mCellBasis[1];
            pXfm->mBasisZ = first.mCellBasis[2];
        }
        const Vector3 point{flLateral - kTrackCenter, 0.0f, 0.0f};
        pXfm->mTranslation =
            LerpRow(XfmPoint(first.mXfm, point), XfmPoint(second.mXfm, point), flBlend);
    } else {
        const TrackGeometry &geometry = mTrackGeometry[nTrack];
        pXfm->mBasisX = geometry.mBasis[0];
        pXfm->mBasisY = geometry.mBasis[1];
        pXfm->mBasisZ = geometry.mBasis[2];
        pXfm->mTranslation.x = ((geometry.mRightX - geometry.mLeftX) * flLateral) + geometry.mLeftX;
        pXfm->mTranslation.z = ((geometry.mRightZ - geometry.mLeftZ) * flLateral) + geometry.mLeftZ;
        pXfm->mTranslation.y = 0.0f;
        Transform path;
        mPath->Xfm(flTick, flTick * *TheGfxManager.mMsPerTick, &path);
        sceVu0MulAffineMatrixXyz(&pXfm->mBasisX.x, &path.mBasisX.x, &pXfm->mBasisX.x);
    }
    if (bTrackOffset) {
        const Vector3 offset = RotateDir(&pXfm->mBasisX, mTrackOffsets[nTrack]);
        pXfm->mTranslation.x += offset.x;
        pXfm->mTranslation.y += offset.y;
        pXfm->mTranslation.z += offset.z;
    }
}

void TnlGeom::PlaceCell(int nTrack,
                        Transform *pXfm,
                        bool bSmoothBasis,
                        bool bTrackOffset,
                        float flTick,
                        float flLateral,
                        float flRaise) {
    CellXfm(nTrack, pXfm, bSmoothBasis, bTrackOffset, flTick, flLateral);
    const float flSink = 1.0f - flRaise;
    pXfm->mTranslation.x += pXfm->mBasisZ.x * flSink;
    pXfm->mTranslation.y += pXfm->mBasisZ.y * flSink;
    pXfm->mTranslation.z += pXfm->mBasisZ.z * flSink;
}

void TnlGeom::BlendCell(Transform *pXfm,
                        bool bSmoothBasis,
                        bool bTrackOffset,
                        float flTrack,
                        float flTick,
                        float flLateral) {
    if (mPath->mSegments[1].mAnim == nullptr) {
        SetIdentity(pXfm);
        return;
    }
    const float flFloor = std::floor(flTrack);
    const int nTrack = static_cast<int>(flFloor);
    const float flBlend = flTrack - flFloor;
    const int nFirst = WrapTrackIndex(nTrack, mNumTracks);
    const int nSecond = WrapTrackIndex(nTrack + 1, mNumTracks);
    Transform first;
    Transform second;
    CellXfm(nFirst, &first, bSmoothBasis, bTrackOffset, flTick, flLateral);
    CellXfm(nSecond, &second, bSmoothBasis, bTrackOffset, flTick, flLateral);
    pXfm->mTranslation = LerpRow(first.mTranslation, second.mTranslation, flBlend);
    pXfm->mBasisZ = LerpRow(first.mBasisZ, second.mBasisZ, flBlend);
    // Yes, the Y axis comes from the first track alone.
    Mat33BuildOrthonormal(&first.mBasisY.x, &pXfm->mBasisZ.x, &pXfm->mBasisX.x);
}

void TnlGeom::PathXfm(Transform *pXfm, float flTick) {
    if (mPath->mSegments[1].mAnim == nullptr) {
        SetIdentity(pXfm);
        return;
    }
    mPath->Xfm(flTick, flTick * *TheGfxManager.mMsPerTick, pXfm);
}

std::vector<TnlGeom::CrossSectionXfm> *TnlGeom::GetSections(int nTrack, int nBar, bool bDirty) {
    PanelData *pPanel = FindPanel(nTrack, nBar);
    if (pPanel == nullptr) {
        return nullptr;
    }
    if (bDirty) {
        pPanel->mDirty = 1;
    }
    return &pPanel->mSections;
}

TnlGeom::PanelData *TnlGeom::GetPanel(int nTrack, int nBar) {
    return FindPanel(nTrack, nBar);
}

void TnlGeom::RestoreSections(int nTrack, int nBar) {
    PanelData *pPanel = FindPanel(nTrack, nBar);
    if (pPanel != nullptr) {
        pPanel->mSections = pPanel->mBaseSections;
        pPanel->mDirty = 1;
    }
}

TnlGeom::PanelData *TnlGeom::FindPanel(int nTrack, int nBar) {
    if (mPanels.empty()) {
        return nullptr;
    }
    const int nSlot = nBar & kRingMask;
    if (mBarRing[nSlot] != nBar) {
        return nullptr;
    }
    return &mPanels[(nTrack * mNumBars) + nSlot];
}

void TnlGeom::SetTrackOffset(int nTrack, const Vector3 *pOffset) {
    mTrackOffsets[nTrack] = *pOffset;
    mTrackOffsetChanged[nTrack] = true;
}

void TnlGeom::SetTrackColor(int nTrack, const Color *pColor) {
    mTracks[nTrack].mColor = *pColor;
}

Color *TnlGeom::GetTrackColor(int nTrack) {
    return &mTracks[nTrack].mColor;
}

void TnlGeom::SetBarValue(int nBar, Rnd::Mat *pMat) {
    if (mBarValues.empty()) {
        return;
    }
    const int nSlot = nBar & kRingMask;
    if (mBarRing[nSlot] == nBar) {
        mBarValues[nSlot] = pMat;
        mBarValuesDirty = 1;
    }
}

void TnlGeom::SetPanelState(int nTrack,
                            int nBar,
                            char bTransparent,
                            Rnd::Mat *pCheckpointMat,
                            int bPickable,
                            bool bHighlighted,
                            const Color *pColor) {
    PanelData *pPanel = FindPanel(nTrack, nBar);
    if (pPanel == nullptr) {
        return;
    }
    pPanel->mDirty = 1;
    pPanel->mTransparent = bTransparent;
    pPanel->mMesh->mZFunc = Rnd::Mesh::kZFuncLessEqual;
    pPanel->mMesh->mZMode =
        (bTransparent == 1) ? Rnd::Mesh::kZModeZReadOnly : Rnd::Mesh::kZModeZReadWrite;
    pPanel->mCheckpointMat = pCheckpointMat;
    pPanel->mPickable = bPickable;
    pPanel->mHighlighted = bHighlighted ? 1 : 0;
    if (pColor != nullptr) {
        pPanel->mColor = *pColor;
    }
    if (bPickable) {
        pPanel->UpdateTriangles();
    }
}

void TnlGeom::SetPanelColor(int nTrack, int nBar, const Color *pColor, bool bHighlighted) {
    PanelData *pPanel = FindPanel(nTrack, nBar);
    if (pPanel == nullptr) {
        return;
    }
    pPanel->mHighlighted = bHighlighted ? 1 : 0;
    if (pColor != nullptr) {
        pPanel->mColor = *pColor;
    }
}

void TnlGeom::SetAllPanelColors(const Color *pColor, bool bHighlighted) {
    Color color;
    if (pColor != nullptr) {
        color = *pColor;
    }
    const int nEndBar = mFirstBar + mNumBars;
    for (int nTrack = 0; nTrack < mNumTracks; ++nTrack) {
        if (pColor == nullptr) {
            color = mTracks[nTrack].mColor;
        }
        for (int nBar = mFirstBar; nBar < nEndBar; ++nBar) {
            const int nSlot = nBar & kRingMask;
            if (mBarRing[nSlot] != nBar) {
                continue;
            }
            PanelData &panel = mPanels[(nTrack * mNumBars) + nSlot];
            panel.mHighlighted = bHighlighted ? 1 : 0;
            panel.mColor = color;
        }
    }
}

Color *TnlGeom::GetPanelColor(int nTrack, int nBar) {
    PanelData *pPanel = FindPanel(nTrack, nBar);
    return (pPanel != nullptr) ? &pPanel->mColor : nullptr;
}

void TnlGeom::SetPanelStartSection(int nTrack, int nBar, char nSection) {
    PanelData *pPanel = FindPanel(nTrack, nBar);
    if (pPanel != nullptr) {
        pPanel->mStartSection = nSection;
        pPanel->mDirty = 1;
    }
}

void TnlGeom::SetTrackOverlay(int nTrack, Rnd::Mat *pMat, int nMode) {
    TrackData &track = mTracks[nTrack];
    track.mOverlayMode = nMode;
    track.mOverlayMat = pMat;
}

void TnlGeom::SetPanelMat(Rnd::Mat *pMat, int nKind) {
    if (nKind < kDefaultMatThreshold) {
        mDefaultMat = pMat;
        return;
    }
    mPanelMats[nKind] = pMat;
}

void TnlGeom::BlendPath(Rnd::TransAnim *pAnim,
                        int bLoop,
                        int bUseTime,
                        float flLength,
                        float flStart,
                        float flFrame,
                        float flLoopStart,
                        float flLoopEnd) {
    mPath->BlendTo(pAnim, bLoop, bUseTime, flLength, flStart, flFrame, flLoopStart, flLoopEnd);
}

void TnlGeom::SetPath(Rnd::TransAnim *pAnim) {
    mPath->Set(pAnim, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f);
}

TnlGeom::PanelData::PanelData()
    : mMesh(nullptr), mCheckpointMat(nullptr), mColor{1.0f, 1.0f, 1.0f, 1.0f}, mHighlighted(0),
      mDirty(1), mPickable(0), mTransparent(1), mStartSection(0) {
}

TnlGeom::PanelData::~PanelData() {
    delete mMesh;
}

void TnlGeom::PanelData::Rebuild(const MeshHolder *pSource, const Vector3 *pOffset, int nRows) {
    const CrossSectionXfm *pEnd = mSections.data() + nRows;
    const CrossSectionXfm *pSection = mSections.data() + mStartSection;
    Rnd::MeshVert *pOut = mMesh->mVertsOwner->mVerts.data();
    const Rnd::MeshVert *pIn = pSource->mMesh->mVertsOwner->mVerts.data();
    char nHold = mStartSection;
    while (pSection != pEnd) {
        for (int i = 0; i < kRowVerts; ++i) {
            const Vector3 point{
                pIn->mPoint.x + pOffset->x, pIn->mPoint.y + pOffset->y, pIn->mPoint.z + pOffset->z};
            pOut->mPoint = XfmPoint(pSection->mXfm, point);
            pOut->mNorm = RotateDir(pSection->mCellBasis, pIn->mNorm);
            pOut->mColor.a = pIn->mColor.a;
            ++pIn;
            ++pOut;
        }
        if (nHold == 0) {
            ++pSection;
        } else {
            --nHold;
        }
    }
    mMesh->SyncChanged(kRebuildSyncMask);
    mDirty = 0;
    UpdateTriangles();
}

void TnlGeom::PanelData::UpdateTriangles() {
    const Transform &first = mSections.front().mXfm;
    const Transform &last = mSections.back().mXfm;
    const auto edge = [](const Transform &xfm, float flSide) {
        return Vector3{(xfm.mBasisX.x * flSide) + xfm.mTranslation.x,
                       (xfm.mBasisX.y * flSide) + xfm.mTranslation.y,
                       (xfm.mBasisX.z * flSide) + xfm.mTranslation.z};
    };
    const Vector3 firstLeft = edge(first, -kTrackCenter);
    const Vector3 firstRight = edge(first, kTrackCenter);
    const Vector3 lastLeft = edge(last, -kTrackCenter);
    const Vector3 lastRight = edge(last, kTrackCenter);
    SetTriangle(&mTriangles[0], firstLeft, firstRight, lastLeft);
    SetTriangle(&mTriangles[1], firstRight, lastLeft, lastRight);
}

bool TnlGeom::PanelData::Hit(const Rnd::Segment *pSegment) const {
    if (!mPickable) {
        return false;
    }
    float flDistance;
    if (Rnd::TestRayAgainstTriangle(
            *pSegment, mTriangles[0], Rnd::Mat::kCullModeNone, &flDistance) != 0) {
        return true;
    }
    return Rnd::TestRayAgainstTriangle(
               *pSegment, mTriangles[1], Rnd::Mat::kCullModeNone, &flDistance) != 0;
}

TnlGeom::TrackData::TrackData() {
    Clear();
}

void TnlGeom::TrackData::Clear() {
    mPlayers = 0;
    mOverlayMat = nullptr;
    mOverlayMode = 0;
}

TnlGeom::MeshHolder::~MeshHolder() {
    if (!sKeepMeshes) {
        delete mMesh;
        mMesh = nullptr;
    }
}

TnlGeom::Player::Player() {
    mTrack = 0;
    mMoveTime = 1.0f;
    mCamSlide = nullptr;
    mHoldCam = 0;
    mActivatorPosition = 0.0f;
    mReserved14 = 0;
    mActivator = nullptr;
    mGeom = nullptr;
    mPosition = 0.0f;
    mVelocity = 0.0f;
    mCamPosition = 0.0f;
    mCamCurve = nullptr;
    mMoveCurve = nullptr;
    mTrailTicks = kPlayerTrailTicks;
}

TnlGeom::Player::~Player() {
    delete mCamCurve;
    delete mMoveCurve;
}

void TnlGeom::Player::Init(TnlGeom *pGeom, char nIndex) {
    mIndex = nIndex;
    mGeom = pGeom;
}

void TnlGeom::Player::Reset() {
    const float flTrack = static_cast<float>(mTrack);
    mHoldCam = 0;
    mMoveTime = 1.0f;
    mVelocity = 0.0f;
    mPosition = flTrack;
    mCamPosition = flTrack;
}

void TnlGeom::Player::Update(float flTick, float flDeltaTicks) {
    const float flDelta = std::fabs(flDeltaTicks);
    float flMoved = 0.0f;
    const float flTrack = static_cast<float>(mTrack);
    if (mPosition != flTrack) {
        if (mMoveCurve->mX1 < flTick) {
            flMoved = flTrack - mPosition;
            mPosition = flTrack;
        } else {
            const float flNumTracks = static_cast<float>(mGeom->mNumTracks);
            const float flPosition = WrapTrack(mMoveCurve->Interp(flTick), flNumTracks);
            flMoved = WrapDistance(flPosition - mPosition, flNumTracks);
            mPosition = flPosition;
        }
    }
    if (1.0f < flDelta) {
        mVelocity = flMoved / flDelta;
    }
    if ((mCamPosition == flTrack) || mHoldCam) {
        return;
    }
    if (mCamCurve->mX1 < flTick) {
        mCamPosition = flTrack;
        return;
    }
    const float flNumTracks = static_cast<float>(mGeom->mNumTracks);
    mCamPosition = WrapTrack(mCamCurve->Interp(flTick), flNumTracks);
}

void TnlGeom::Player::SetCamSlide(Rnd::Transformable *pSlide) {
    mCamSlide = pSlide;
}

void TnlGeom::Player::SetTrack(char nTrack) {
    const unsigned char bit = static_cast<unsigned char>(1 << mIndex);
    mGeom->mTracks[mTrack].mPlayers &= static_cast<unsigned char>(~bit);
    mGeom->mTracks[nTrack].mPlayers |= bit;
    if (nTrack == mTrack) {
        return;
    }
    mTrack = nTrack;
    const float flTrack = static_cast<float>(nTrack);
    if (!mGeom->mAnimateMoves) {
        mPosition = flTrack;
        mVelocity = 0.0f;
        mCamPosition = flTrack;
        return;
    }

    const float flNow = TheGameDb->mSongTick;
    const float flNumTracks = static_cast<float>(mGeom->mNumTracks);
    if (mGeom->mWraps) {
        const float flDistance = WrapDistance(flTrack - mCamPosition, flNumTracks);
        InvExpInterpolator curve(mCamPosition,
                                 mCamPosition + flDistance,
                                 flNow,
                                 flNow + (mMoveTime / mGeom->mCamMoveSpeed),
                                 kMovePower);
        if (mCamCurve != nullptr) {
            mCamCurve->Resample(curve, kCamCurveSamples);
        } else {
            mCamCurve = new TableLinInterpolator(curve, kCamCurveSamples);
        }
    } else {
        const float flScale = (std::fabs(flTrack - mCamPosition) * 0.5f) + 0.5f;
        InvExpInterpolator curve(mCamPosition,
                                 flTrack,
                                 flNow,
                                 flNow + ((mMoveTime * flScale) / mGeom->mCamMoveSpeed),
                                 kMovePower);
        if (mCamCurve != nullptr) {
            mCamCurve->Resample(curve, kCamCurveSamples);
        } else {
            mCamCurve = new TableLinInterpolator(curve, kCamCurveSamples);
        }
    }

    if (mGeom->mWraps) {
        const float flDistance = WrapDistance(flTrack - mPosition, flNumTracks);
        InvExpInterpolator curve(mPosition,
                                 mPosition + flDistance,
                                 flNow,
                                 flNow + (mMoveTime * mGeom->mMoveLength),
                                 kMovePower);
        if (mMoveCurve != nullptr) {
            mMoveCurve->Resample(curve, kMoveCurveSamples);
        } else {
            mMoveCurve = new TableLinInterpolator(curve, kMoveCurveSamples);
        }
    } else {
        const float flLength = mGeom->MoveLength(mPosition, flTrack);
        InvExpInterpolator curve(
            mPosition, flTrack, flNow, flNow + (mMoveTime * flLength), kMovePower);
        if (mMoveCurve != nullptr) {
            mMoveCurve->Resample(curve, kMoveCurveSamples);
        } else {
            mMoveCurve = new TableLinInterpolator(curve, kMoveCurveSamples);
        }
    }
}

void TnlGeom::Player::LeaveTrack() {
    mGeom->mTracks[mTrack].mPlayers &= static_cast<unsigned char>(~(1 << mIndex));
}

void TnlGeom::Player::SetActivator(Rnd::View *pView) {
    mActivator = pView;
}

void TnlGeom::Player::SetActivatorPosition(float flPosition) {
    mActivatorPosition = flPosition;
}

void TnlGeom::Player::PlaceActivator(const Transform *pXfm) {
    if (mActivator == nullptr) {
        return;
    }
    mActivator->SetLocalXfm(*pXfm);
    mActivator->UpdateWorldXfm(nullptr, 0);
}
