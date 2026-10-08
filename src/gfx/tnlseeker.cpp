#include "gfx/tnlseeker.h"

#include <cmath>
#include <utility>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "math/transform.h"
#include "os/string.h"
#include "os/timer.h"
#include "rnd/manager.h"
#include "rnd/view.h"

namespace {

constexpr float kNoTick = -1e9f;
constexpr float kNoTrack = -1.0f;
constexpr char kNoTargetTrack = -1;
constexpr float kNoDrawnTrack = -2.0f;
constexpr int kNoCount = -1;

// The glide curves ease with this power.
constexpr float kGlidePower = 3.0f;

// A solo game draws every frame with the materials of this letter.
constexpr char kSoloLetter = 's';

// A distance across the tracks that rounds to the whole tunnel is one track.
constexpr float kTrackRounding = 0.4999f;
// A glide shorter than this in total takes no time to scale.
constexpr float kMinGlideDistance = 0.001f;

// The frame is not drawn ahead of the song by more than kMaxLeadTicks, and not behind it by more
// than kMaxTrailTicks.
constexpr float kMaxLeadTicks = 15360.0f;
constexpr float kMaxTrailTicks = 960.0f;

// Every part of the mesh changes when the frame moves, except the texture coordinates.
constexpr int kSyncShape = Rnd::Mesh::kSyncTexs - 1;

// The sizes of the mesh beyond those of the divisions of the rails. Vertex 0 of a quarter is the
// inner corner of the far prong.
constexpr int kFarProngCorner = 0;
constexpr int kExtraSectionVerts = 12;
constexpr int kExtraQuads = 44;
constexpr int kQuadsPerDivision = 8;

// Texture coordinates of the frame.
constexpr float kTexEdge = 0.0f;
constexpr float kTexRim = 0.1f;
constexpr float kTexProng = 0.6f;
constexpr float kTexInner = 0.9f;
constexpr float kTexFull = 1.0f;
constexpr float kTexFarStart = 0.2f;
constexpr float kTexFarEnd = 0.3f;
constexpr float kTexNear = 0.8f;
constexpr float kTexNearEnd = 0.7f;

const char *const kFrameSectionName = "seeker_frame";

// NTSC-U/C: 0x003afc30
float sProngWidth = 0.2f;
// NTSC-U/C: 0x003afc34
float sFrameWidth = 0.1f;
// NTSC-U/C: 0x003afc38
float sFrameHeight = 0.1f;
// NTSC-U/C: 0x003afc3c
float sMultiFrameHeight = 0.3f;
// NTSC-U/C: 0x003afc40
float sProngThickness = 60.0f;
// NTSC-U/C: 0x003afc44
int sDivisions = 1;
// NTSC-U/C: 0x0043b6f0
float sInnerProngWidth = sProngWidth - sFrameWidth;

} // namespace

float TnlSeeker::sFadeSpeed = 0.005f;

TnlSeeker::TnlSeeker(TnlGeom *pGeom, int, const char *pszColor)
    : mGeom(pGeom), mTrackCurve(new InvExpInterpolator(0.0f, 0.0f, 0.0f, 1.0f, kGlidePower)),
      mDepthCurve(new InvExpInterpolator(0.0f, 0.0f, 0.0f, 1.0f, kGlidePower)),
      mFromTrack(kNoTrack), mTrack(kNoTrack), mTargetTrack(kNoTargetTrack),
      mDrawnTrack(kNoDrawnTrack), mFromStartTick(0.0f), mStartTick(0.0f), mTargetStartTick(0.0f),
      mDrawnStartTick(kNoTick), mFromEndTick(0.0f), mEndTick(0.0f), mTargetEndTick(0.0f),
      mDrawnEndTick(kNoTick), mSectionVerts(kNoCount), mNumQuads(kNoCount), mDirty(0),
      mMultiplier(0), mMultiplierStartTime(kNoTick), mMultiplierFade(0.0f, 0.0f, 0.0f, 1.0f) {
    const char nLetter =
        (TheGameDb->mCommunity == GameDb::kCommunitySolo) ? kSoloLetter : pszColor[0];
    mMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("seeker_%c.mat", nLetter)));
    mMultiplierMat =
        dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("seeker mult_%c.mat", nLetter)));
    mMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Create(
        Rnd::Mesh::sClassName.mStr, FormatString("seeker %s.mesh", pszColor)));
    mMesh->SetMat(mMat);
    mMesh->mVertsOwner->mMutable = 1;
    dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("tnl opaque"))->AddDraw(mMesh, nullptr);
    mMesh->SetShowing(0);
}

TnlSeeker::~TnlSeeker() {
    delete mMesh;
    delete mDepthCurve;
    delete mTrackCurve;
}

void TnlSeeker::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    DataArray *pFrame;
    FindConfigArray(pConfig, pDefaults, kFrameSectionName, &pFrame, true);
    pFrame->FindFloat("prong_width", &sProngWidth, true);
    pFrame->FindFloat("frame_width", &sFrameWidth, true);
    pFrame->FindFloat("frame_height", &sFrameHeight, true);
    pFrame->FindFloat("multi_frame_height", &sMultiFrameHeight, true);
    pFrame->FindFloat("inner_prong_width", &sInnerProngWidth, true);
    pFrame->FindFloat("prong_thickness", &sProngThickness, true);
    DataArray *pDivisions = pFrame->FindArray("divisions", true);
    int nCommunity;
    switch (TheGameDb->mCommunity) {
    case GameDb::kCommunityLocal:
    case GameDb::kCommunityOnline:
        nCommunity = TheGameDb->mCommunity;
        break;
    default:
        nCommunity = GameDb::kCommunitySolo;
        break;
    }
    sDivisions = pDivisions->Int(nCommunity);
    sFrameHeight *= TheGfxManager.mScrollSpeed;
    sMultiFrameHeight *= TheGfxManager.mScrollSpeed;

    mSectionVerts = (sDivisions * 2) + kExtraSectionVerts;
    std::vector<Rnd::MeshVert> &verts = mMesh->mVertsOwner->mVerts;
    verts = std::vector<Rnd::MeshVert>();
    Rnd::MeshVert vert;
    vert.mPoint = Vector3{0.0f, 0.0f, 0.0f};
    vert.mNorm = Vector3{0.0f, 0.0f, 0.0f};
    vert.mColor = Color{1.0f, 1.0f, 1.0f, 1.0f};
    vert.mTex1 = Vector2{0.0f, 0.0f};
    vert.mTex2 = Vector2{0.0f, 0.0f};
    verts.resize(mSectionVerts * 4, vert);

    mNumQuads = (sDivisions * kQuadsPerDivision) + kExtraQuads;
    std::vector<Rnd::MeshFace> &faces = mMesh->mFacesOwner->mFaces;
    faces = std::vector<Rnd::MeshFace>();
    faces.reserve(mNumQuads * 2);

    // n is the number of divisions and N the vertices of a quarter. The quarters are the inner
    // and outer layers of the two sides, at 0, N, 2N, and 3N.
    const int n = sDivisions;
    const int N = mSectionVerts;
    const int nFarRim = (n * 2) + 11;
    const int nFarInner = (n * 2) + 9;
    const int nFarProng = (n * 2) + 8;
    const int nRailEnd = n + 7;
    const int nNearRim = n + 3;
    const int nNearInner = n + 4;
    const int nNearProng = n + 5;
    const int nNearTip = n + 6;
    const int nRailStart = n + 2;
    const int nRailSpans = n + 1;
    const int nSecondSide = N * 2;
    const int nSecondOuter = N * 3;

    AddStrip(faces, kFarProngCorner, nFarRim, 1, -1, 1, false);
    AddStrip(faces, nFarProng, 1, 1, nFarInner, 1, false);
    AddStrip(faces, N, N + nFarRim, 1, -1, 1, true);
    AddStrip(faces, N + nFarProng, N + 1, 1, nFarInner, 1, true);
    AddStrip(faces, nNearInner, nNearRim, 1, -1, 1, false);
    AddStrip(faces, nNearTip, nNearProng, 1, -3, 1, false);
    AddStrip(faces, N + nNearInner, N + nNearRim, 1, -1, 1, true);
    AddStrip(faces, N + nNearTip, N + nNearProng, 1, -3, 1, true);
    AddStrip(faces, nRailEnd, nRailStart, 1, -1, nRailSpans, false);
    AddStrip(faces, N + nRailEnd, N + nRailStart, 1, -1, nRailSpans, true);
    AddStrip(faces, N, kFarProngCorner, 1, 1, nFarRim, false);
    AddStrip(faces, N + nFarRim, nFarRim, -nFarRim, -nFarRim, 1, false);
    AddStrip(faces, nSecondSide + nFarRim, nSecondSide, -1, 1, 1, false);
    AddStrip(faces, nSecondSide + 1, nSecondSide + nFarProng, nFarInner, 1, 1, false);
    AddStrip(faces, nSecondOuter + nFarRim, nSecondOuter, -1, 1, 1, true);
    AddStrip(faces, nSecondOuter + 1, nSecondOuter + nFarProng, nFarInner, 1, 1, true);
    AddStrip(faces, nSecondSide + nNearRim, nSecondSide + nNearInner, -1, 1, 1, false);
    AddStrip(faces, nSecondSide + nNearProng, nSecondSide + nNearTip, -3, 1, 1, false);
    AddStrip(faces, nSecondOuter + nNearRim, nSecondOuter + nNearInner, -1, 1, 1, true);
    AddStrip(faces, nSecondOuter + nNearProng, nSecondOuter + nNearTip, -3, 1, 1, true);
    AddStrip(faces, nSecondSide + nRailStart, nSecondSide + nRailEnd, -1, 1, nRailSpans, false);
    AddStrip(faces, nSecondOuter + nRailStart, nSecondOuter + nRailEnd, -1, 1, nRailSpans, true);
    AddStrip(faces, nSecondSide, nSecondOuter, 1, 1, nFarRim, false);
    AddStrip(faces, nSecondSide + nFarRim, nSecondOuter + nFarRim, -nFarRim, -nFarRim, 1, false);

    SetTexCoords(verts, kFarProngCorner, kTexInner, kTexFarStart, kTexFull, kTexFarEnd);
    SetTexCoords(verts, nFarRim, kTexInner, kTexRim, kTexFull, kTexEdge);
    SetTexCoords(verts, (n * 2) + 10, kTexProng, kTexRim, kTexProng, kTexEdge);
    SetTexCoords(verts, nFarInner, kTexRim, kTexRim, kTexEdge, kTexEdge);
    SetTexCoords(verts, nNearRim, kTexInner, kTexNear, kTexFull, kTexNearEnd);
    SetTexCoords(verts, nNearInner, kTexInner, kTexInner, kTexFull, kTexFull);
    SetTexCoords(verts, nNearProng, kTexProng, kTexInner, kTexProng, kTexFull);
    SetTexCoords(verts, nNearTip, kTexRim, kTexInner, kTexEdge, kTexFull);
    const float flRailStep = -kTexProng / static_cast<float>(sDivisions + 1);
    float flRailV = kTexNear;
    int nRail = nRailStart;
    int nRailInner = nRailEnd;
    while (nRail > 0) {
        SetTexCoords(verts, nRail--, kTexProng, flRailV, kTexNearEnd, flRailV);
        SetTexCoords(verts, nRailInner++, kTexRim, flRailV, kTexEdge, flRailV);
        flRailV += flRailStep;
    }
    mMesh->SyncChanged(Rnd::Mesh::kSyncTexs);
}

void TnlSeeker::AddStrip(std::vector<Rnd::MeshFace> &faces,
                         int nRowA,
                         int nRowB,
                         int nStepA,
                         int nStepB,
                         int nCount,
                         bool bFlip) {
    if (bFlip) {
        std::swap(nRowA, nRowB);
        std::swap(nStepA, nStepB);
    }
    for (int i = 0; i < nCount; ++i) {
        const int nNextA = nRowA + nStepA;
        const int nNextB = nRowB + nStepB;
        faces.push_back(Rnd::MeshFace{static_cast<unsigned short>(nRowA),
                                      static_cast<unsigned short>(nRowB),
                                      static_cast<unsigned short>(nNextA)});
        faces.push_back(Rnd::MeshFace{static_cast<unsigned short>(nNextA),
                                      static_cast<unsigned short>(nRowB),
                                      static_cast<unsigned short>(nNextB)});
        nRowA = nNextA;
        nRowB = nNextB;
    }
}

void TnlSeeker::SetTexCoords(std::vector<Rnd::MeshVert> &verts,
                             int nIndex,
                             float flInnerU,
                             float flInnerV,
                             float flOuterU,
                             float flOuterV) {
    const Vector2 inner{flInnerU, flInnerV};
    const Vector2 outer{flOuterU, flOuterV};
    const int nSecondSide = mSectionVerts * 2;
    verts[nIndex].mTex1 = inner;
    verts[nSecondSide + nIndex].mTex1 = inner;
    verts[mSectionVerts + nIndex].mTex1 = outer;
    verts[nSecondSide + mSectionVerts + nIndex].mTex1 = outer;
}

void TnlSeeker::SetSection(std::vector<Rnd::MeshVert> &verts,
                           int nIndex,
                           float flTrack,
                           float flLateral,
                           float flTick,
                           float flHeight) {
    const int nSecondSide = mSectionVerts * 2;
    Transform xfm;

    mGeom->BlendCell(&xfm, false, true, flTrack, flTick, flLateral);
    Rnd::MeshVert &outer = verts[mSectionVerts + nIndex];
    outer.mPoint = xfm.mTranslation;
    outer.mNorm = xfm.mBasisZ;
    Rnd::MeshVert &inner = verts[nIndex];
    inner.mNorm = xfm.mBasisZ;
    inner.mPoint.x = xfm.mTranslation.x + (xfm.mBasisZ.x * flHeight);
    inner.mPoint.y = xfm.mTranslation.y + (xfm.mBasisZ.y * flHeight);
    inner.mPoint.z = xfm.mTranslation.z + (xfm.mBasisZ.z * flHeight);

    mGeom->BlendCell(&xfm, false, true, flTrack, flTick, 1.0f - flLateral);
    Rnd::MeshVert &otherOuter = verts[nSecondSide + mSectionVerts + nIndex];
    otherOuter.mPoint = xfm.mTranslation;
    otherOuter.mNorm = xfm.mBasisZ;
    Rnd::MeshVert &otherInner = verts[nSecondSide + nIndex];
    otherInner.mNorm = xfm.mBasisZ;
    otherInner.mPoint.x = xfm.mTranslation.x + (xfm.mBasisZ.x * flHeight);
    otherInner.mPoint.y = xfm.mTranslation.y + (xfm.mBasisZ.y * flHeight);
    otherInner.mPoint.z = xfm.mTranslation.z + (xfm.mBasisZ.z * flHeight);
}

void TnlSeeker::Layout(std::vector<Rnd::MeshVert> &verts,
                       float flTrack,
                       float flStartTick,
                       float flEndTick) {
    Timer timer;
    timer.Start();
    if (flStartTick < flEndTick) {
        const float flCutoff = TheGameDb->mSongTick - kMaxTrailTicks;
        const int n = sDivisions;
        float flHeight;
        if (mMultiplier) {
            const float flFade = mMultiplierFade.Interp(TheGameDb->mSongTick);
            flHeight = ((sFrameHeight - sMultiFrameHeight) * flFade) + sMultiFrameHeight;
        } else {
            flHeight = sFrameHeight;
        }
        const float flNarrowProng = (sFrameWidth < sProngWidth) ? sFrameWidth : sProngWidth;

        float flTick = flEndTick - sProngThickness;
        if (flCutoff < flTick) {
            SetSection(verts, 0, flTrack, sFrameWidth + sInnerProngWidth, flTick, flHeight);
        }
        if (flCutoff < flEndTick) {
            SetSection(verts, (n * 2) + 11, flTrack, sProngWidth, flEndTick, flHeight);
        }
        if (flCutoff < flEndTick) {
            SetSection(verts, (n * 2) + 10, flTrack, flNarrowProng, flEndTick, flHeight);
        }
        if (flCutoff < flEndTick) {
            SetSection(verts, (n * 2) + 9, flTrack, 0.0f, flEndTick, flHeight);
        }
        flTick = flStartTick + sProngThickness;
        if (flCutoff < flTick) {
            SetSection(verts, n + 3, flTrack, sFrameWidth + sInnerProngWidth, flTick, flHeight);
        }
        if (flCutoff < flStartTick) {
            SetSection(verts, n + 4, flTrack, sProngWidth, flStartTick, flHeight);
        }
        if (flCutoff < flStartTick) {
            SetSection(verts, n + 5, flTrack, flNarrowProng, flStartTick, flHeight);
        }
        if (flCutoff < flStartTick) {
            SetSection(verts, n + 6, flTrack, 0.0f, flStartTick, flHeight);
        }

        const float flRailEnd = flEndTick - sProngThickness;
        float flRailTick = flStartTick + sProngThickness;
        const float flRailStep = (flRailEnd - flRailTick) / static_cast<float>(sDivisions + 1);
        int nRail = n + 2;
        int nRailInner = n + 7;
        for (; nRail > 0; --nRail, ++nRailInner, flRailTick += flRailStep) {
            if (flCutoff < flRailTick) {
                SetSection(verts, nRail, flTrack, sFrameWidth, flRailTick, flHeight);
            }
            if (flCutoff < flRailTick) {
                SetSection(verts, nRailInner, flTrack, 0.0f, flRailTick, flHeight);
            }
        }
    }
    timer.Stop();
}

void TnlSeeker::SetTarget(char nTrack, float flStartTick, float flEndTick) {
    mDirty = 0;
    float flTrack;
    if (mFromTrack == kNoTrack) {
        flTrack = static_cast<float>(nTrack);
        mFromEndTick = flStartTick; // Yes, the original starts both ends at the start tick.
        mFromStartTick = flStartTick;
        mFromTrack = flTrack;
    } else {
        flTrack = static_cast<float>(nTrack);
        mFromTrack = mTrack;
        mFromStartTick = mStartTick;
        mFromEndTick = mEndTick;
    }
    mTargetTrack = nTrack;
    mTargetStartTick = flStartTick;
    mTargetEndTick = flEndTick;

    const float flEndDistance = std::fabs(mGeom->mBarsPerTick * (flEndTick - mFromEndTick));
    const float flStartDistance = std::fabs(mGeom->mBarsPerTick * (flStartTick - mFromStartTick));
    const float flTimeScale = 2.0f / sFadeSpeed;
    float flTrackDistance = std::fabs(flTrack - mFromTrack);
    const float flDepthDistance =
        (flStartDistance < flEndDistance) ? flEndDistance : flStartDistance;
    if (static_cast<int>(flTrackDistance + kTrackRounding) == mGeom->mNumTracks) {
        flTrackDistance = 1.0f;
    }
    const float flTotal = flTrackDistance + flDepthDistance;
    float flInvTotal = 1.0f;
    if (!(flTotal < kMinGlideDistance)) {
        flInvTotal = 1.0f / flTotal;
    }
    const float flTrackTime = flTrackDistance * flInvTotal * flTimeScale;
    const float flDepthTime = flDepthDistance * flInvTotal * flTimeScale;
    const float flNow = TheGameDb->mSongTime;
    const float flDepthStart = flNow + flTrackTime;
    mTrackCurve->Reset(mFromTrack, flTrack, flNow, flDepthStart);
    mDepthCurve->Reset(0.0f, 1.0f, flDepthStart, flDepthStart + flDepthTime);
}

void TnlSeeker::StartMultiplier(float flEndTick) {
    mMultiplierStartTime = TheGameDb->mSongTime;
    const float flTick = TheGameDb->mSongTick;
    if (flEndTick <= flTick) {
        return;
    }
    mMultiplierFade.Reset(0.0f, 1.0f, flTick, flEndTick);
    mMesh->SetMat(mMultiplierMat);
    mMultiplier = 1;
}

void TnlSeeker::StopMultiplier() {
    mMultiplier = 0;
    mMesh->SetMat(mMat);
}

void TnlSeeker::SetWindow(char, float, float) {
}

void TnlSeeker::OnRangeChanged(const TnlTrackRange *pRange) {
    const bool bOverlaps = pRange->Overlaps(
        static_cast<char>(static_cast<int>(mDrawnTrack)), mDrawnStartTick, mDrawnEndTick);
    mDirty = (mDirty != 0) || bOverlaps;
}

void TnlSeeker::Poll(float, float) {
    const float flNow = TheGameDb->mSongTime;
    const float flTick = TheGameDb->mSongTick;
    mTrack = mTrackCurve->Eval(flNow);
    const float flDepth = mDepthCurve->Eval(flNow);
    mEndTick = ((mTargetEndTick - mFromEndTick) * flDepth) + mFromEndTick;
    mStartTick = ((mTargetStartTick - mFromStartTick) * flDepth) + mFromStartTick;
    const float flLimit = flTick + kMaxLeadTicks;
    if (flLimit <= mEndTick) {
        mStartTick = flLimit;
        mEndTick = flLimit;
    }

    if (mMultiplier || mStartTick != mDrawnStartTick || mEndTick != mDrawnEndTick ||
        mTrack != mDrawnTrack || mDirty) {
        mDirty = 0;
        if (mStartTick < mEndTick) {
            mDrawnStartTick = mStartTick;
            mDrawnEndTick = mEndTick;
            mDrawnTrack = mTrack;
            if (kMaxTrailTicks < flTick - mEndTick) {
                mMesh->SetShowing(0);
            } else {
                Layout(mMesh->mVertsOwner->mVerts, mTrack, mStartTick, mEndTick);
                mMesh->SyncChanged(kSyncShape);
                mMesh->SetShowing(1);
            }
        } else {
            mMesh->SetShowing(0);
        }
    }

    if (mMultiplier && mMultiplierFade.mX1 <= flTick) {
        mMultiplier = 0;
        mMesh->SetMat(mMat);
    }
}
