#include "rnd/psparticlesys.h"

#include <cstring>

#include "rnd/dmapacket.h"
#include "rnd/ps.h"
#include "rnd/pscam.h"
#include "rnd/psmat.h"

namespace {

// ZBUF_1 masks depth writes, and TEST_1 passes every pixel or only those nearer than the depth
// buffer.
constexpr int kGsRegZbuf1 = 0x4e;
constexpr int kGsRegTest1 = 0x47;
constexpr unsigned long long kZbufZmsk = 1ULL << 32;
constexpr unsigned long long kTestZTestAlways = 0x20000;
constexpr unsigned long long kTestZTestGreater = 0x60000;
constexpr unsigned long long kTestZTestMask = 0x60000;

// VIF codes. UNPACK V4-32 to an address relative to the double buffer, then MSCAL or MSCNT.
constexpr unsigned int kVifUnpackV4 = 0x6c000000;
constexpr unsigned int kVifUnpackFlg = 0x8000;
constexpr int kVifUnpackNumShift = 16;
constexpr unsigned int kVifMscalSprite = 0x14000512;
constexpr unsigned int kVifMscnt = 0x17000000;
constexpr int kVifCodeWord = 3;

// The constants of the microprogram, the projection, the colour scale, the GIF tag, the viewport
// scale and offset, and the sprite scale.
constexpr int kConstantQwords = 9;

// GIF tag of the sprites, six registers of ST, RGBAQ, and XYZF2 for the two corners, with a
// sprite primitive that is Gouraud shaded and, as selected, textured and blended.
constexpr unsigned long long kSpriteGifTag = 0x6000400000008000ULL;
constexpr unsigned long long kSpriteGifRegs = 0x412412;
constexpr unsigned long long kPrimSpriteGouraud = 0xe;
constexpr int kPrimTmeShift = 4;
constexpr int kPrimAbeShift = 6;
constexpr int kGifTagPrimShift = 47;

// A batch is at most 162 particles of two quadwords, and an unpack at most 254 quadwords.
constexpr int kMaxBatchParticles = 0xa2;
constexpr int kMaxUnpackQwords = 0xfe;
constexpr int kParticleQwords = 2;

constexpr float kHalf = 0.5f;

constexpr int kLowDword = 0;
constexpr int kHighDword = 1;

inline void PushQuadword(unsigned long long qwLow, unsigned long long qwHigh) {
    DmaPacket &packet = DmaPacket::shared();
    packet.GetCursor()->mDword[kLowDword] = qwLow;
    packet.GetCursor()->mDword[kHighDword] = qwHigh;
    ++packet.GetCursor();
}

inline void PushFloats(float fX, float fY, float fZ, float fW) {
    DmaPacket &packet = DmaPacket::shared();
    const float afValues[] = {fX, fY, fZ, fW};
    std::memcpy(packet.GetCursor(), afValues, sizeof(afValues));
    ++packet.GetCursor();
}

inline void PushVector(const Vector3 &vector) {
    PushFloats(vector.x, vector.y, vector.z, vector.w);
}

inline void PushColor(const Color &color) {
    PushFloats(color.r, color.g, color.b, color.a);
}

// A quadword of three VIF NOPs and one VIF code.
inline void PushVifCode(unsigned int nCode) {
    DmaPacket &packet = DmaPacket::shared();
    packet.GetCursor()->mWord[0] = 0;
    packet.GetCursor()->mWord[1] = 0;
    packet.GetCursor()->mWord[2] = 0;
    packet.GetCursor()->mWord[kVifCodeWord] = nCode;
    ++packet.GetCursor();
}

inline void ClearQuadword(DmaQuadword *pQuadword) {
    pQuadword->mDword[kLowDword] = 0;
    pQuadword->mDword[kHighDword] = 0;
}

inline unsigned int UnpackCode(int nAddress, int nQwords) {
    return static_cast<unsigned int>(nAddress) |
           (static_cast<unsigned int>(nQwords) << kVifUnpackNumShift) | kVifUnpackFlg |
           kVifUnpackV4;
}

} // namespace

PsParticleSys::PsParticleSys(const char *pszName) : RndObject(pszName), RndParticleSys(pszName) {
}

int PsParticleSys::DrawShowing() {
    if (mLiveParticles == nullptr || mMode != kModeSprite) {
        return 1;
    }
    int nPass = 0;
    ThePs.SetGsReg(kGsRegZbuf1, kZbufZmsk, kZbufZmsk);
    ThePs.SetGsReg(kGsRegTest1, mReadZ != 0 ? kTestZTestGreater : kTestZTestAlways, kTestZTestMask);
    do {
        if (mMat != nullptr) {
            nPass = static_cast<PsMat *>(mMat)->Select(nPass);
        } else {
            PsMat::SelectDefault();
        }
        DrawParticles();
    } while (nPass != 0);
    return 1;
}

void PsParticleSys::DrawParticles() {
    DmaPacket &frame = DmaPacket::shared();
    frame.CloseGifTag(1, 0);

    PushVifCode(UnpackCode(0, kConstantQwords));
    PushVector(g_viewProjectXfm.mBasisX);
    PushVector(g_viewProjectXfm.mBasisY);
    PushVector(g_viewProjectXfm.mBasisZ);
    PushVector(g_viewProjectXfm.mTranslation);
    PushColor(g_colorScale);
    const unsigned long long qwPrim =
        kPrimSpriteGouraud | (static_cast<unsigned long long>(g_nTextured) << kPrimTmeShift) |
        (static_cast<unsigned long long>(g_nAlphaBlendEnabled) << kPrimAbeShift);
    PushQuadword((qwPrim << kGifTagPrimShift) | kSpriteGifTag, kSpriteGifRegs);
    PushFloats(g_viewportXfm.mBasisX.x, g_viewportXfm.mBasisY.y, g_viewportXfm.mBasisZ.z, 0.0f);
    PushFloats(g_viewportXfm.mTranslation.x,
               g_viewportXfm.mTranslation.y,
               g_viewportXfm.mTranslation.z,
               0.0f);
    PushVector(g_particleProjectScale);

    bool bFirst = true;
    Particle *pParticle = mLiveParticles;
    while (pParticle != nullptr) {
        DmaQuadword *pUnpack = frame.GetCursor();
        ClearQuadword(pUnpack);
        ++frame.GetCursor();
        DmaQuadword *pCount = frame.GetCursor();
        ClearQuadword(pCount);
        ++frame.GetCursor();

        int nAddress = kConstantQwords;
        int nQwords = 1;
        int nCount = 0;
        do {
            if (nQwords >= kMaxUnpackQwords) {
                pUnpack->mWord[kVifCodeWord] = UnpackCode(nAddress, nQwords);
                nAddress += nQwords;
                nQwords = 0;
                pUnpack = frame.GetCursor();
                ClearQuadword(pUnpack);
                ++frame.GetCursor();
            }
            ++nCount;
            nQwords += kParticleQwords;
            pParticle->mPos.w = pParticle->mSize * kHalf;
            PushColor(pParticle->mCol);
            PushVector(pParticle->mPos);
            pParticle = pParticle->mNext;
        } while (nCount < kMaxBatchParticles && pParticle != nullptr);
        pUnpack->mWord[kVifCodeWord] = UnpackCode(nAddress, nQwords);
        pCount->mWord[0] = static_cast<unsigned int>(nCount);

        PushVifCode(bFirst ? kVifMscalSprite : kVifMscnt);
        bFirst = false;
        ThePs.Flush();
    }
}
