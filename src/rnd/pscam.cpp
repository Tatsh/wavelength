#include "rnd/pscam.h"

#include <cmath>

#include "math/plane.h"
#include "rnd/dmapacket.h"
#include "rnd/ps.h"
#include "rnd/pstex.h"
#include "rnd/rndmanager.h"

namespace {

// Centre of the GS primitive coordinate space, as a float for the guard band and as an integer for
// the sprite that clears a render target, and the four fractional bits of a primitive coordinate.
constexpr float kGsCoordinateCentre = 2048.0f;
constexpr int kGsCoordinateCentreInt = 0x800;
constexpr int kGsSubpixelShift = 4;

constexpr float kHalf = 0.5f;

// Fraction of the screen rectangle each guard band axis is measured against, a 2 per cent margin
// over half the extent.
constexpr float kGuardBandHalfExtent = 0.51f;

// The clip space depth runs from -1 to 1.
constexpr float kClipDepthSpan = 2.0f;
constexpr int kBitsPerByte = 8;

// The GS registers DrawShowing() writes, and the fields it sets.
constexpr int kGsRegTexFlush = 0x3f;
constexpr int kGsRegScissor1 = 0x40;
constexpr int kGsRegTest1 = 0x47;
constexpr int kGsRegFba1 = 0x4a;
constexpr int kGsRegZbuf1 = 0x4e;
constexpr unsigned long long kAllBits = ~0ULL;
constexpr unsigned long long kTestAte = 1;
constexpr unsigned long long kFbaFba = 1;
constexpr unsigned long long kZbufZmsk = 1ULL << 32;
constexpr unsigned long long kTestZTestMask = 0x60000;
constexpr unsigned long long kTestZTestAlways = 0x20000;
constexpr int kScissorX1Shift = 16;
constexpr int kScissorY0Shift = 32;
constexpr int kScissorY1Shift = 48;

// GIF tags. An A+D tag sends one register per quadword, and the clear sprite sends PRIM, RGBAQ,
// and two XYZ2 as a register list.
constexpr int kGifFlgPacked = 0;
constexpr int kGifFlgRegList = 1;
constexpr int kGifAdRegs = 1;
constexpr unsigned long long kGifRegAd = 0xe;
constexpr int kSpriteRegs = 4;
constexpr unsigned long long kSpriteRegList = 0x5510;
constexpr unsigned long long kPrimSprite = 6;
constexpr int kGsYShift = 16;

constexpr int kLowDword = 0;
constexpr int kHighDword = 1;

// Clamp a coordinate of the screen rectangle to the unit range.
inline float ClampToUnit(float fValue) {
    if (1.0f < fValue) {
        return 1.0f;
    }
    if (fValue < 0.0f) {
        return 0.0f;
    }
    return fValue;
}

// Widen one side plane of the local frustum by a guard band factor applied to the y component of
// its normal, retaining the point of the plane nearest the origin.
inline void WidenSidePlane(const Plane &plane, float fScale, Plane &out) {
    float fNormalX = plane.a;
    float fNormalY = plane.b * fScale;
    float fNormalZ = plane.c;
    const float fInvLength =
        1.0f / std::sqrt(fNormalX * fNormalX + fNormalY * fNormalY + fNormalZ * fNormalZ);
    fNormalX *= fInvLength;
    fNormalY *= fInvLength;
    fNormalZ *= fInvLength;

    const float fDistance = -plane.d;
    const float fPointX = fDistance * plane.a;
    const float fPointY = fDistance * plane.b;
    const float fPointZ = fDistance * plane.c;
    out.a = fNormalX;
    out.b = fNormalY;
    out.c = fNormalZ;
    out.d = -(fNormalX * fPointX + fNormalY * fPointY + fNormalZ * fPointZ);
}

// Move a plane from camera space to world space in place.
inline void TransformPlane(Plane &plane, const Transform &xfm) {
    const float fA = xfm.mBasisX.x * plane.a + xfm.mBasisY.x * plane.b + xfm.mBasisZ.x * plane.c;
    const float fB = xfm.mBasisX.y * plane.a + xfm.mBasisY.y * plane.b + xfm.mBasisZ.y * plane.c;
    const float fC = xfm.mBasisX.z * plane.a + xfm.mBasisY.z * plane.b + xfm.mBasisZ.z * plane.c;
    plane.a = fA;
    plane.b = fB;
    plane.c = fC;
    plane.d = plane.d - (xfm.mTranslation.x * plane.a + xfm.mTranslation.y * plane.b +
                         xfm.mTranslation.z * plane.c);
}

inline void PushQuadword(unsigned long long qwLow, unsigned long long qwHigh) {
    DmaPacket &packet = DmaPacket::shared();
    packet.mCursor->mDword[kLowDword] = qwLow;
    packet.mCursor->mDword[kHighDword] = qwHigh;
    ++packet.mCursor;
}

inline unsigned long long PackXy(int nX, int nY) {
    return static_cast<unsigned long long>(nX) | (static_cast<unsigned long long>(nY) << kGsYShift);
}

} // namespace

alignas(16) Vector3 g_invGuardBandScale;
alignas(16) Vector3 g_particleProjectScale;
alignas(16) Frustum g_drawFrustum;
alignas(16) Transform g_viewProjectUnscaledXfm;
alignas(16) Transform g_viewProjectXfm;
alignas(16) Transform g_viewProjectBiasedXfm;
alignas(16) Transform g_viewportUnscaledXfm;
alignas(16) Transform g_viewportXfm;

PsCam::PsCam(const char *pszName) : RndObject(pszName), RndCam(pszName) {
}

PsCam::~PsCam() {
}

int PsCam::DrawShowing() {
    const bool bPreviousTarget = sCurrent != nullptr && sCurrent->mTargetTex != nullptr;
    if (bPreviousTarget) {
        DmaPacket::shared().OpenGifTag(kGifFlgPacked, kGifAdRegs, kGifRegAd, 0);
        PushQuadword(0, kGsRegTexFlush);
    }

    int nTargetWidth;
    int nTargetHeight;
    if (mTargetTex != nullptr) {
        static_cast<PsTex *>(mTargetTex)->BindAsRenderTarget();
        nTargetHeight = mTargetTex->mHeight;
        nTargetWidth = mTargetTex->mWidth;
    } else {
        if (bPreviousTarget) {
            ThePs.RestoreFrameBufferTarget();
        }
        nTargetHeight = ThePs.mScreenHeight;
        nTargetWidth = ThePs.mScreenWidth;
    }
    const float fWidth = static_cast<float>(nTargetWidth);
    const float fHeight = static_cast<float>(nTargetHeight);

    const float fCentreX = mScreenRect.x + mScreenRect.w * kHalf;
    const float fCentreY = mScreenRect.y + mScreenRect.h * kHalf;
    const float fOffsetX = fCentreX - kHalf;
    const float fOffsetY = fCentreY - kHalf;
    const float fGuardX = (kGsCoordinateCentre - std::fabs(fOffsetX * fWidth)) /
                          (mScreenRect.w * fWidth * kGuardBandHalfExtent);
    const float fGuardY = (kGsCoordinateCentre - std::fabs(fOffsetY * fHeight)) /
                          (mScreenRect.h * fHeight * kGuardBandHalfExtent);
    g_invGuardBandScale.z = 1.0f;
    g_invGuardBandScale.x = 1.0f / fGuardX;
    g_invGuardBandScale.y = 1.0f / fGuardY;

    g_drawFrustum.mFront = mLocalFrustum.mFront;
    g_drawFrustum.mBack = mLocalFrustum.mBack;
    WidenSidePlane(mLocalFrustum.mLeft, fGuardX, g_drawFrustum.mLeft);
    WidenSidePlane(mLocalFrustum.mRight, fGuardX, g_drawFrustum.mRight);
    WidenSidePlane(mLocalFrustum.mTop, fGuardY, g_drawFrustum.mTop);
    WidenSidePlane(mLocalFrustum.mBottom, fGuardY, g_drawFrustum.mBottom);
    TransformPlane(g_drawFrustum.mFront, mWorldXfm);
    TransformPlane(g_drawFrustum.mBack, mWorldXfm);
    TransformPlane(g_drawFrustum.mLeft, mWorldXfm);
    TransformPlane(g_drawFrustum.mRight, mWorldXfm);
    TransformPlane(g_drawFrustum.mTop, mWorldXfm);
    TransformPlane(g_drawFrustum.mBottom, mWorldXfm);

    // The three basis rows are set to the identity without touching their padding words.
    Transform &viewport = g_viewportUnscaledXfm;
    viewport.mBasisX.x = 1.0f;
    viewport.mBasisX.y = 0.0f;
    viewport.mBasisX.z = 0.0f;
    viewport.mBasisY.x = 0.0f;
    viewport.mBasisY.y = 1.0f;
    viewport.mBasisY.z = 0.0f;
    viewport.mBasisZ.x = 0.0f;
    viewport.mBasisZ.y = 0.0f;
    viewport.mBasisZ.z = 1.0f;
    viewport.mTranslation.x = 0.0f;
    viewport.mTranslation.y = 0.0f;
    viewport.mTranslation.z = 0.0f;
    viewport.mTranslation.w = 1.0f;

    const int nMaxDepth = (1 << (ThePs.mDepthBytes * kBitsPerByte)) - 1;
    const float fHalfDepth = static_cast<float>(nMaxDepth) * kHalf;
    viewport.mBasisX.x = fWidth * mScreenRect.w * kHalf;
    viewport.mBasisY.y = fHeight * mScreenRect.h * kHalf;
    viewport.mTranslation.x = fOffsetX * fWidth + kGsCoordinateCentre;
    viewport.mTranslation.y = fOffsetY * fHeight + kGsCoordinateCentre;
    viewport.mBasisZ.z = fHalfDepth * (mZRange.x - mZRange.y);
    g_viewportXfm.mBasisX = viewport.mBasisX;
    g_viewportXfm.mBasisY = viewport.mBasisY;
    g_viewportXfm.mBasisX.x = g_viewportXfm.mBasisX.x * fGuardX;
    g_viewportXfm.mBasisY.y = g_viewportXfm.mBasisY.y * fGuardY;
    g_viewportXfm.mBasisZ = viewport.mBasisZ;
    viewport.mTranslation.z = fHalfDepth * (kClipDepthSpan - mZRange.x - mZRange.y);
    g_viewportXfm.mTranslation = viewport.mTranslation;

    Transform &project = g_viewProjectUnscaledXfm;
    project.mBasisX = mLocalProject.mBasisX;
    project.mBasisY = mLocalProject.mBasisY;
    project.mBasisZ = mLocalProject.mBasisZ;
    project.mTranslation = mLocalProject.mTranslation;
    project.mBasisZ.w = 0.0f;
    project.mBasisY.w = 0.0f;
    project.mBasisX.w = 0.0f;
    project.mTranslation.w = 1.0f;
    if (mFov == 0.0f) {
        project.mBasisY.z = kClipDepthSpan / (mFarPlane - mNearPlane);
        project.mTranslation.z = 1.0f - mFarPlane * project.mBasisY.z;
    } else {
        project.mTranslation.w = 0.0f;
        project.mBasisY.w = project.mBasisY.z;
        project.mBasisY.z = (mFarPlane + mNearPlane) / (mFarPlane - mNearPlane);
        project.mTranslation.z = mFarPlane - mFarPlane * project.mBasisY.z;
    }

    g_viewProjectXfm.mBasisX = project.mBasisX;
    g_viewProjectXfm.mBasisZ = project.mBasisZ;
    g_viewProjectBiasedXfm.mBasisY = project.mBasisY;
    g_viewProjectBiasedXfm.mTranslation = project.mTranslation;
    g_viewProjectXfm.mBasisY = project.mBasisY;
    g_viewProjectXfm.mTranslation = project.mTranslation;
    g_viewProjectXfm.mBasisX.x = g_viewProjectXfm.mBasisX.x / fGuardX;
    g_viewProjectXfm.mBasisZ.y = g_viewProjectXfm.mBasisZ.y / fGuardY;
    g_viewProjectBiasedXfm.mBasisX = g_viewProjectXfm.mBasisX;
    g_viewProjectBiasedXfm.mBasisZ = g_viewProjectXfm.mBasisZ;
    if (mFov != 0.0f) {
        // One depth unit, in clip space.
        const float fBias =
            kClipDepthSpan / (static_cast<float>(nMaxDepth) * (mZRange.y - mZRange.x));
        g_viewProjectBiasedXfm.mBasisY.z =
            (mFarPlane + mNearPlane + fBias * (mNearPlane - mFarPlane)) / (mFarPlane - mNearPlane);
        g_viewProjectBiasedXfm.mTranslation.z =
            mFarPlane * (1.0f - fBias) - mFarPlane * project.mBasisY.z;
    }

    g_particleProjectScale.x = g_viewProjectXfm.mBasisX.x;
    g_particleProjectScale.z = 0.0f;
    g_particleProjectScale.y = -g_viewProjectXfm.mBasisZ.y;
    g_particleProjectScale.w = 0.0f;
    sceVu0MulAffineMatrix(&project.mBasisX.x, &project.mBasisX.x, &mInvWorldXfm.mBasisX.x);
    sceVu0MulAffineMatrix(
        &g_viewProjectXfm.mBasisX.x, &g_viewProjectXfm.mBasisX.x, &mInvWorldXfm.mBasisX.x);
    sceVu0MulAffineMatrix(&g_viewProjectBiasedXfm.mBasisX.x,
                          &g_viewProjectBiasedXfm.mBasisX.x,
                          &mInvWorldXfm.mBasisX.x);

    const int nX0 = static_cast<int>(ClampToUnit(mScreenRect.x) * fWidth);
    const int nX1 = static_cast<int>(ClampToUnit(mScreenRect.x + mScreenRect.w) * fWidth) - 1;
    const int nY0 = static_cast<int>(ClampToUnit(mScreenRect.y) * fHeight);
    const int nY1 = static_cast<int>(ClampToUnit(mScreenRect.y + mScreenRect.h) * fHeight) - 1;
    ThePs.SetGsReg(kGsRegScissor1,
                   static_cast<unsigned long long>(nX0) |
                       (static_cast<unsigned long long>(nX1) << kScissorX1Shift) |
                       (static_cast<unsigned long long>(nY0) << kScissorY0Shift) |
                       (static_cast<unsigned long long>(nY1) << kScissorY1Shift),
                   kAllBits);

    if (mTargetTex != nullptr) {
        ThePs.SetGsReg(kGsRegTest1, 0, kTestAte);
        ThePs.SetGsReg(kGsRegFba1, 0, kFbaFba);
        ThePs.SetGsReg(kGsRegZbuf1, 0, kZbufZmsk);
        ThePs.SetGsReg(kGsRegTest1, kTestZTestAlways, kTestZTestMask);
        DmaPacket::shared().OpenGifTag(kGifFlgRegList, kSpriteRegs, kSpriteRegList, 0);
        PushQuadword(kPrimSprite, 0);
        const int nLeft = (kGsCoordinateCentreInt - (nTargetWidth >> 1)) << kGsSubpixelShift;
        const int nTop = (kGsCoordinateCentreInt - (nTargetHeight >> 1)) << kGsSubpixelShift;
        PushQuadword(PackXy(nLeft, nTop),
                     PackXy(nLeft + (nTargetWidth << kGsSubpixelShift),
                            nTop + (nTargetHeight << kGsSubpixelShift)));
    }
    sCurrent = this;
    return 1;
}

void PsCam::Init() {
    TheManager.mClasses[RndCam::sClassName] = PsCam::New;
    RndCam::CreateDefault();
}

// The unit's static initialiser at NTSC-U/C: 0x00213548, PAL: 0x0021c360, and its global
// constructor at NTSC-U/C: 0x00213550, PAL: 0x0021c368, are empty.
