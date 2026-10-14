#include "rnd/psenviron.h"

#include <cmath>
#include <cstring>

#include "rnd/dmapacket.h"
#include "rnd/ps.h"
#include "rnd/psmat.h"
#include "rnd/rndlight.h"
#include "rnd/rndmanager.h"

namespace {

// The fog value of no fog, and the scale from a colour component to a GS byte.
constexpr float kFogValueClear = 255.0f;
constexpr float kByteScale = 255.0f;

constexpr int kGsRegFogCol = 0x3d;
constexpr unsigned long long kFogColMask = 0xffffff;
constexpr int kFogColGreenShift = 8;
constexpr int kFogColBlueShift = 16;

// One point light, or up to three directional lights.
constexpr int kMaxDirectionalLights = 3;

// The words of the header quadword of a lighting block, and the size of the block.
constexpr int kHeaderProgram = 0;
constexpr int kHeaderVertBase = 1;
constexpr int kHeaderVertLight = 2;
constexpr int kHeaderReserved = 3;
constexpr int kLightBlockQwords = 8;
constexpr int kAmbientTailQwords = 6;
constexpr int kPointTailQwords = 2;

constexpr int kLowDword = 0;
constexpr int kHighDword = 1;

// The components of a quadword vector.
enum Component {
    kComponentX = 0,
    kComponentY = 1,
    kComponentZ = 2,
};

inline void PushFloats(float fX, float fY, float fZ, float fW) {
    DmaPacket &packet = DmaPacket::shared();
    const float afValues[] = {fX, fY, fZ, fW};
    std::memcpy(packet.mCursor, afValues, sizeof(afValues));
    ++packet.mCursor;
}

inline void PushColor(const Color &color) {
    PushFloats(color.r, color.g, color.b, color.a);
}

inline void PushVector(const Vector3 &vector) {
    PushFloats(vector.x, vector.y, vector.z, vector.w);
}

inline void PushProduct(const Color &first, const Color &second) {
    PushFloats(first.r * second.r, first.g * second.g, first.b * second.b, first.a * second.a);
}

inline float &ComponentOf(Vector3 &vector, int nComponent) {
    switch (nComponent) {
    case kComponentX:
        return vector.x;
    case kComponentY:
        return vector.y;
    default:
        return vector.z;
    }
}

// The lighting block of an object without lights to send, the ambient program.
inline void SendAmbient(DmaQuadword *pHeader, const PsMat *pMat) {
    pHeader->mWord[kHeaderProgram] = PsEnviron::kLightProgramAmbient;
    pHeader->mWord[kHeaderVertBase] = pMat->mVertBase;
    DmaPacket::shared().mCursor += kAmbientTailQwords;
}

// Each light direction, a column of g_lightMatrix, moved into the space of a basis row.
inline Vector3 LightsAlong(const Vector3 &row) {
    Vector3 out;
    out.x = g_lightMatrix[0].x * row.x + g_lightMatrix[1].x * row.y + g_lightMatrix[2].x * row.z;
    out.y = g_lightMatrix[0].y * row.x + g_lightMatrix[1].y * row.y + g_lightMatrix[2].y * row.z;
    out.z = g_lightMatrix[0].z * row.x + g_lightMatrix[1].z * row.y + g_lightMatrix[2].z * row.z;
    out.w = row.w;
    return out;
}

} // namespace

float g_fogScale;
float g_fogOffset = kFogValueClear;
int g_nFogEnabled;
PsEnviron::LightProgram g_lightProgram = PsEnviron::kLightProgramUnlit;
alignas(16) Color g_ambientColor;
alignas(16) Vector3 g_lightMatrix[3];
alignas(16) Color g_lightColors[3];

// The unit's static initialiser runs an empty constructor over the three elements of a light
// array, and its global constructor calls the static initialiser.
// NTSC-U/C: 0x00219ec0, PAL: 0x00222cd8
// NTSC-U/C: 0x00219f08, PAL: 0x00222d20

int PsEnviron::DrawShowing() {
    g_nFogEnabled = mFogMode != kFogNone;
    if (g_nFogEnabled) {
        g_fogScale = kFogValueClear / (mFogStart - mFogEnd);
        g_fogOffset = -mFogEnd * g_fogScale;
    } else {
        g_fogScale = 0.0f;
        g_fogOffset = kFogValueClear;
    }
    const auto nRed = static_cast<unsigned long long>(static_cast<int>(mFogColor.r * kByteScale));
    const auto nGreen = static_cast<unsigned long long>(static_cast<int>(mFogColor.g * kByteScale));
    const auto nBlue = static_cast<unsigned long long>(static_cast<int>(mFogColor.b * kByteScale));
    ThePs.SetGsReg(kGsRegFogCol,
                   nRed | (nGreen << kFogColGreenShift) | (nBlue << kFogColBlueShift),
                   kFogColMask);

    g_ambientColor.r = mAmbientColor.r;
    g_ambientColor.g = mAmbientColor.g;
    g_ambientColor.b = mAmbientColor.b;
    g_ambientColor.a = 1.0f;
    g_lightProgram = kLightProgramAmbient;

    auto point = mLights.begin();
    while (point != mLights.end() && (*point)->mType != RndLight::kTypePoint) {
        ++point;
    }
    if (point != mLights.end()) {
        const RndLight *pLight = *point;
        g_lightProgram = kLightProgramPoint;
        g_lightMatrix[0] = pLight->mWorldXfm.mTranslation;
        g_lightMatrix[0].w = -1.0f / pLight->mRange;
        g_lightMatrix[1].w = pLight->mRange * pLight->mRange;
        g_lightMatrix[2].w = pLight->mRange;
        g_lightColors[0] = pLight->mColor;
        g_lightColors[0].a = 0.0f;
    } else {
        int nLights = 0;
        for (auto it = mLights.begin(); it != mLights.end() && nLights < kMaxDirectionalLights;
             ++it) {
            const RndLight *pLight = *it;
            if (pLight->mType != RndLight::kTypeDirectional) {
                continue;
            }
            g_lightProgram = kLightProgramDirectional;
            const Vector3 &dir = pLight->mWorldXfm.mBasisY;
            const float fScale = 1.0f / std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
            ComponentOf(g_lightMatrix[0], nLights) = -(dir.x * fScale);
            ComponentOf(g_lightMatrix[1], nLights) = -(dir.y * fScale);
            ComponentOf(g_lightMatrix[2], nLights) = -(dir.z * fScale);
            g_lightColors[nLights] = pLight->mColor;
            g_lightColors[nLights].a = 0.0f;
            ++nLights;
        }
        if (g_lightProgram == kLightProgramDirectional) {
            for (; nLights < kMaxDirectionalLights; ++nLights) {
                g_lightColors[nLights] = Color{0.0f, 0.0f, 0.0f, 0.0f};
            }
        }
    }

    sCurrent = this;
    return 1;
}

void PsEnviron::SelectLights(const Transform *pXfm, const Sphere *pSphere) {
    DmaPacket &frame = DmaPacket::shared();
    DmaQuadword *pHeader = frame.mCursor;
    if (g_nLightingEnabled == 0) {
        pHeader->mWord[kHeaderProgram] = kLightProgramUnlit;
        pHeader->mWord[kHeaderVertBase] = 0;
        pHeader->mWord[kHeaderVertLight] = 0;
        pHeader->mWord[kHeaderReserved] = 0;
        frame.mCursor += kLightBlockQwords;
        return;
    }

    const PsMat *pMat = PsMat::sCurrent;
    pHeader->mDword[kLowDword] = 0;
    pHeader->mDword[kHighDword] = 0;
    ++frame.mCursor;
    if (pMat->mVertBase != 0) {
        if (pMat->mBaseAmbient != 0) {
            PushColor(g_ambientColor);
        } else if (g_lightProgram == kLightProgramAmbient) {
            pHeader->mWord[kHeaderProgram] = kLightProgramUnlit;
            frame.mCursor += kLightBlockQwords - 1;
            return;
        } else {
            PushFloats(1.0f, 1.0f, 1.0f, 1.0f);
        }
    } else if (pMat->mBaseAmbient != 0) {
        PushProduct(pMat->mBaseColor, g_ambientColor);
    } else {
        PushColor(pMat->mBaseColor);
    }

    if (pXfm == nullptr || g_lightProgram == kLightProgramAmbient) {
        SendAmbient(pHeader, pMat);
        return;
    }

    if (g_lightProgram == kLightProgramDirectional) {
        pHeader->mWord[kHeaderProgram] = kLightProgramDirectional;
        pHeader->mWord[kHeaderVertBase] = pMat->mVertBase;
        pHeader->mWord[kHeaderVertLight] = pMat->mVertLight;
        const Vector3 rowX = LightsAlong(pXfm->mBasisX);
        const Vector3 rowY = LightsAlong(pXfm->mBasisY);
        const Vector3 rowZ = LightsAlong(pXfm->mBasisZ);
        PushVector(rowX);
        PushVector(rowY);
        PushVector(rowZ);
        for (const Color &color : g_lightColors) {
            if (pMat->mVertLight == 0) {
                PushProduct(pMat->mLightColor, color);
            } else {
                PushColor(color);
            }
        }
        return;
    }

    Transform inverse;
    Invert(inverse, *pXfm);
    Vector3 position;
    MultiplyPoint(inverse, g_lightMatrix[0], position);
    if (pSphere->mRadius != 0.0f) {
        const float fX = position.x - pSphere->mCenter.x;
        const float fY = position.y - pSphere->mCenter.y;
        const float fZ = position.z - pSphere->mCenter.z;
        const float fReach = g_lightMatrix[2].w + pSphere->mRadius;
        if (fReach * fReach < fX * fX + fY * fY + fZ * fZ) {
            SendAmbient(pHeader, pMat);
            return;
        }
    }
    pHeader->mWord[kHeaderProgram] = kLightProgramPoint;
    pHeader->mWord[kHeaderVertBase] = pMat->mVertBase;
    pHeader->mWord[kHeaderVertLight] = pMat->mVertLight;
    PushFloats(position.x, 0.0f, 0.0f, g_lightMatrix[0].w);
    PushFloats(position.y, 0.0f, 0.0f, g_lightMatrix[1].w);
    PushFloats(position.z, 0.0f, 0.0f, 0.0f);
    if (pMat->mVertLight == 0) {
        PushProduct(pMat->mLightColor, g_lightColors[0]);
    } else {
        PushColor(g_lightColors[0]);
    }
    frame.mCursor += kPointTailQwords;
}

void PsEnviron::Init() {
    TheManager.mClasses[RndEnviron::sClassName] = PsEnviron::New;
    RndEnviron::CreateDefault();
    RndLight::CreateDefault();
}

void PsEnviron::Terminate() {
    TheManager.mClasses[RndEnviron::sClassName] = RndEnviron::New;
}
