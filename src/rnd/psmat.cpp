#include "rnd/psmat.h"

#include "rnd/ps.h"
#include "rnd/pstex.h"
#include "rnd/rndmanager.h"

namespace {

// The GS registers a material programs.
constexpr int kGsRegClamp1 = 0x08;
constexpr int kGsRegAlpha1 = 0x42;
constexpr int kGsRegDimX = 0x44;
constexpr int kGsRegTest1 = 0x47;
constexpr int kGsRegFba1 = 0x4a;

// ALPHA_1 equations as A, B, C, and D fields, (A - B) * C + D, where 0 is the source colour, 1 the
// destination colour, and 2 zero or, for C, the fixed alpha. The fixed alpha of 128 is 1.0.
constexpr unsigned long long kAlphaKeepDest = 0x4a;
constexpr unsigned long long kAlphaAddFix = 0x68;
constexpr unsigned long long kAlphaSubtractFix = 0xa1;
constexpr unsigned long long kAlphaBlendSrc = 0x44;
constexpr unsigned long long kAlphaAddSrcAlpha = 0x48;
constexpr unsigned long long kAlphaBlendInvSrc = 0x01;
constexpr unsigned long long kAlphaBlendDest = 0x54;
constexpr unsigned long long kAlphaBlendInvDest = 0x11;
constexpr unsigned long long kAlphaScaleSrc = 0x88;
constexpr unsigned long long kAlphaFixOne = 0x80ULL << 32;
constexpr unsigned long long kAlphaMask = 0xff;
constexpr unsigned long long kAlphaFixMask = 0xff000000ffULL;

// DIMX dither matrices, one for blends that add to the destination and one for the rest. The mask
// covers the three bits of each entry.
constexpr unsigned long long kDimXAdditive = 0x1212303012120303ULL;
constexpr unsigned long long kDimXDefault = 0x2435607135247160ULL;
constexpr unsigned long long kDimXMask = 0x7777777777777777ULL;

// TEST_1 settings. An alpha-blended surface passes pixels of alpha above 0 and, failing that,
// still writes depth. A cutout passes the same pixels and writes nothing for the others.
constexpr unsigned long long kTestAlphaGreaterZOnly = 0x200d;
constexpr unsigned long long kTestAlphaGreater = 0xd;
constexpr unsigned long long kTestAlphaMask = 0x3fff;
constexpr unsigned long long kTestAte = 1;
constexpr unsigned long long kFbaFba = 1;

// CLAMP_1 wrap modes on both axes, and the mask of the two mode fields.
constexpr unsigned long long kClampBothAxes = 5;
constexpr unsigned long long kRepeatBothAxes = 0;
constexpr unsigned long long kClampMask = 0xf;

// The colour scales. A modulated texture halves the vertex colour range.
constexpr float kColorScaleFull = 255.0f;
constexpr float kColorScaleModulate = 128.0f;
constexpr float kAlphaScale = 128.0f;

// The TEX0 texture functions.
constexpr int kTexFunctionModulate = 0;
constexpr int kTexFunctionDecal = 1;
constexpr int kTexFunctionHighlight = 2;
constexpr int kTexFunctionHighlight2 = 3;

// The second and later passes draw their texture as a decal, or as a second highlight when the
// material draws two extra passes.
constexpr int kMultiPassDouble = 2;
constexpr int kStageCountShared = 2;

// The sphere map moves the texture coordinates from the unit square centred on 0 to the unit
// square from 0 to 1, flipping v.
constexpr float kSphereMapScale = 0.5f;
constexpr float kTexelCentreOffset = 0.5f;

// NTSC-U/C: 0x003b0788, PAL: 0x0042f620
const int kTexFunctions[] = {
    kTexFunctionModulate,
    kTexFunctionDecal,
    kTexFunctionHighlight2,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionHighlight,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionModulate,
    kTexFunctionModulate,
};

// Set the first three rows to the identity without touching their padding words, and the
// translation row to the origin.
inline void SetIdentityUvXfm() {
    g_uvXfm.mBasisZ.z = 1.0f;
    g_uvXfm.mBasisX.x = 1.0f;
    g_uvXfm.mBasisY.z = 0.0f;
    g_uvXfm.mBasisX.y = 0.0f;
    g_uvXfm.mBasisX.z = 0.0f;
    g_uvXfm.mBasisY.x = 0.0f;
    g_uvXfm.mBasisY.y = 1.0f;
    g_uvXfm.mBasisZ.x = 0.0f;
    g_uvXfm.mBasisZ.y = 0.0f;
    g_uvXfm.mTranslation.x = 0.0f;
    g_uvXfm.mTranslation.y = 0.0f;
    g_uvXfm.mTranslation.z = 0.0f;
    g_uvXfm.mTranslation.w = 1.0f;
}

inline void SetColorScale(float fScale) {
    g_colorScale.r = fScale;
    g_colorScale.a = kAlphaScale;
    g_colorScale.b = fScale;
    g_colorScale.g = fScale;
}

} // namespace

PsMat *PsMat::sCurrent;
int g_nSelectedGenMode;
int g_nSelectedFlat;
int g_nTextured;
int g_nAlphaBlendEnabled;
int g_nLightingEnabled;
int g_nReselectBlend;
alignas(16) Color g_colorScale;
alignas(16) Transform g_uvXfm;
alignas(16) Vector3 g_stageXfm[3];

void PsMat::SelectDefault() {
    SetColorScale(kColorScaleFull);
    g_nAlphaBlendEnabled = 1;
    g_nSelectedFlat = 0;
    g_nTextured = 0;
    sCurrent = nullptr;
    g_nLightingEnabled = 0;
    g_nReselectBlend = 0;
    ThePs.SetGsReg(kGsRegAlpha1, kAlphaBlendSrc, kAlphaMask);
    ThePs.SetGsReg(kGsRegFba1, kFbaFba, kFbaFba);
    ThePs.SetGsReg(kGsRegDimX, kDimXDefault, kDimXMask);
}

void PsMat::SelectAlphaBlend() {
    ThePs.SetGsReg(kGsRegAlpha1, kAlphaBlendSrc, kAlphaMask);
    g_nReselectBlend = 1;
}

int PsMat::Select(int nPass) {
    if (this == sCurrent && mStages.size() < kStageCountShared) {
        if (g_nReselectBlend != 0) {
            SelectBlendMode(nPass);
            g_nReselectBlend = 0;
        }
        return 0;
    }
    g_nSelectedFlat = mFlat;
    SelectBlendMode(nPass);
    BindStageTexture(nPass);
    SetupUvXfm(nPass);
    sCurrent = this;
    const unsigned int nNext = nPass + 1;
    return nNext < mStages.size() ? static_cast<int>(nNext) : 0;
}

void PsMat::SelectBlendMode(int nPass) {
    const int nBlend = nPass == 0 ? mBlend : mStages[nPass].mBlend;
    switch (nBlend) {
    case kBlendDest:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaKeepDest, kAlphaMask);
        break;
    case kBlendSrc:
    case kBlendMultiply:
    case kBlendMultiply2:
    case kBlendSrcAdd:
        g_nAlphaBlendEnabled = 0;
        break;
    case kBlendAdd:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaFixOne | kAlphaAddFix, kAlphaFixMask);
        break;
    case kBlendSrcAlpha:
    case kBlendSrcAlphaCutout:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaBlendSrc, kAlphaMask);
        break;
    case kBlendSrcAlphaAdd:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaAddSrcAlpha, kAlphaMask);
        break;
    case kBlendInvSrcAlpha:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaBlendInvSrc, kAlphaMask);
        break;
    case kBlendDestAlpha:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaBlendDest, kAlphaMask);
        break;
    case kBlendInvDestAlpha:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaBlendInvDest, kAlphaMask);
        break;
    case kBlendSrcAlphaOpaque:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaScaleSrc, kAlphaMask);
        break;
    case kBlendSubtract:
        g_nAlphaBlendEnabled = 1;
        ThePs.SetGsReg(kGsRegAlpha1, kAlphaFixOne | kAlphaSubtractFix, kAlphaFixMask);
        break;
    default:
        break;
    }

    if (nBlend == kBlendAdd || nBlend == kBlendDest || nBlend == kBlendSrcAlphaAdd ||
        nBlend == kBlendSubtract) {
        ThePs.SetGsReg(kGsRegDimX, kDimXAdditive, kDimXMask);
    } else {
        ThePs.SetGsReg(kGsRegDimX, kDimXDefault, kDimXMask);
    }

    if (nBlend == kBlendSrcAlpha) {
        ThePs.SetGsReg(kGsRegTest1, kTestAlphaGreaterZOnly, kTestAlphaMask);
    } else if (nBlend == kBlendSrcAlphaCutout) {
        ThePs.SetGsReg(kGsRegTest1, kTestAlphaGreater, kTestAlphaMask);
    } else {
        ThePs.SetGsReg(kGsRegTest1, 0, kTestAte);
    }
    ThePs.SetGsReg(kGsRegFba1, nBlend != kBlendSrc, kFbaFba);
}

void PsMat::SelectStageClamp(const Stage &stage) {
    if (stage.mWrap == kTexWrapClamp) {
        ThePs.SetGsReg(kGsRegClamp1, kClampBothAxes, kClampMask);
    } else {
        ThePs.SetGsReg(kGsRegClamp1, kRepeatBothAxes, kClampMask);
    }
}

int PsMat::TexFunction(int nBlend) {
    return kTexFunctions[nBlend];
}

void PsMat::BindStageTexture(int nPass) {
    g_nLightingEnabled = mEnable;
    if (mStages.empty() || mStages[nPass].mTex == nullptr || mStages[nPass].mBlend == kBlendDest) {
        g_colorScale.r = kColorScaleFull;
        g_colorScale.a = kAlphaScale;
        g_nTextured = 0;
        g_colorScale.b = kColorScaleFull;
        g_colorScale.g = kColorScaleFull;
        return;
    }
    Stage &stage = mStages[nPass];
    int nBlend;
    if (nPass == 0) {
        nBlend = stage.mBlend;
    } else {
        nBlend = mMultiPass == kMultiPassDouble ? kBlendMultiply : kBlendSrc;
    }
    static_cast<PsTex *>(stage.mTex)->Select(TexFunction(nBlend));
    if (nBlend == kBlendSrc) {
        g_nLightingEnabled = 0;
    }
    g_nTextured = 1;
    if (nBlend == kBlendMultiply2) {
        SetColorScale(kColorScaleFull);
    } else {
        SetColorScale(kColorScaleModulate);
    }
    SelectStageClamp(stage);
}

void PsMat::SetupUvXfm(int nPass) {
    if (g_nTextured == 0) {
        SetIdentityUvXfm();
        return;
    }
    const Stage &stage = mStages[nPass];
    g_nSelectedGenMode = stage.mGenMode;
    if (stage.mGenMode == kTexGenSphere) {
        SetIdentityUvXfm();
        g_uvXfm.mTranslation.y = kSphereMapScale;
        g_uvXfm.mBasisX.x = kSphereMapScale;
        g_uvXfm.mBasisY.y = -kSphereMapScale;
        g_uvXfm.mTranslation.x = kSphereMapScale;
        if (stage.mUseXfm != 0) {
            g_stageXfm[0] = stage.mXfm.mBasisX;
            g_stageXfm[1] = stage.mXfm.mBasisY;
            g_stageXfm[2] = stage.mXfm.mBasisZ;
            return;
        }
        g_stageXfm[0].x = 1.0f;
        g_stageXfm[2].z = 1.0f;
        g_stageXfm[1].z = 0.0f;
        g_stageXfm[2].x = 0.0f;
        g_stageXfm[0].y = 0.0f;
        g_stageXfm[0].z = 0.0f;
        g_stageXfm[1].x = 0.0f;
        g_stageXfm[1].y = 1.0f;
        g_stageXfm[2].y = 0.0f;
        return;
    }
    if (stage.mUseXfm == 0) {
        SetIdentityUvXfm();
        return;
    }
    // The stage transform is applied about the texel centre, with v flipped.
    const Transform &xfm = stage.mXfm;
    const float fOffsetU = -xfm.mTranslation.x - kTexelCentreOffset;
    const float fOffsetV = xfm.mTranslation.y - kTexelCentreOffset;
    g_uvXfm.mBasisX.x = xfm.mBasisX.x;
    g_uvXfm.mBasisX.y = -xfm.mBasisX.y;
    g_uvXfm.mBasisY.x = -xfm.mBasisY.x;
    g_uvXfm.mBasisY.y = xfm.mBasisY.y;
    g_uvXfm.mTranslation.x =
        fOffsetU * g_uvXfm.mBasisX.x + fOffsetV * g_uvXfm.mBasisY.x + kTexelCentreOffset;
    g_uvXfm.mTranslation.y =
        fOffsetU * g_uvXfm.mBasisX.y + fOffsetV * g_uvXfm.mBasisY.y + kTexelCentreOffset;
}

void PsMat::Init() {
    TheManager.mClasses[RndMat::sClassName] = PsMat::New;
}

// The unit's static initialiser at NTSC-U/C: 0x00213f40, PAL: 0x0021cd58, and its global
// constructor at NTSC-U/C: 0x00213f48, PAL: 0x0021cd60, are empty.
