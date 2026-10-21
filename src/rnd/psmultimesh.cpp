#include "rnd/psmultimesh.h"

#include <cstdint>
#include <cstring>

#include <eekernel.h>

#include "os/memhandle.h"
#include "rnd/dmapacket.h"
#include "rnd/ps.h"
#include "rnd/pscam.h"
#include "rnd/psenviron.h"
#include "rnd/psmat.h"
#include "rnd/psmesh.h"

namespace {

// VIF codes. UNPACK V4-32 to an address relative to the double buffer, then MSCAL or MSCNT.
constexpr unsigned int kVifUnpackV4 = 0x6c000000;
constexpr unsigned int kVifUnpackFlg = 0x8000;
constexpr int kVifUnpackNumShift = 16;
constexpr unsigned int kVifMscalInstanced = 0x140003dc;
constexpr unsigned int kVifMscnt = 0x17000000;
constexpr int kVifCodeWord = 3;

// The constants of the microprogram, the projection, the colour scale, the viewport scale and
// offset, the texture coordinate transform, the GIF tag, and the lighting block.
constexpr int kConstantQwords = 0x13;

// GIF tag of the faces, nine registers of ST, RGBAQ, and XYZF2 for the three corners, with a
// triangle primitive that is Gouraud shaded unless flat and, as selected, textured and blended.
constexpr unsigned long long kFaceGifTag = 0x9000400000008000ULL;
constexpr unsigned long long kFaceGifRegs = 0x412412412ULL;
constexpr unsigned long long kPrimTriangle = 3;
constexpr int kPrimIipShift = 3;
constexpr int kPrimTmeShift = 4;
constexpr int kPrimAbeShift = 6;
constexpr int kGifTagPrimShift = 47;

// DMA tags. A REF tag of a transform moves its four quadwords from where it is.
constexpr int kDmaTagIdCnt = 1;
constexpr int kDmaTagIdRef = 3;
constexpr unsigned long long kDmaRefTransformTag = 0x30000004;
constexpr int kDmaTagAddressShift = 32;
constexpr int kDmaChannelToSpr = 9;

// The instances unpack after the face data, below the end of the VU1 data buffer, at most 254
// quadwords at a time.
constexpr int kVuDataEnd = 0x14e;
constexpr int kMaxUnpackQwords = 0xfe;
constexpr int kTransformQwords = 4;

constexpr int kLowDword = 0;
constexpr int kHighDword = 1;

inline void PushQuadword(unsigned long long qwLow, unsigned long long qwHigh) {
    DmaPacket &packet = DmaPacket::shared();
    packet.GetCursor()->mDword[kLowDword] = qwLow;
    packet.GetCursor()->mDword[kHighDword] = qwHigh;
    packet.Advance();
}

inline void PushFloats(float fX, float fY, float fZ, float fW) {
    DmaPacket &packet = DmaPacket::shared();
    const float afValues[] = {fX, fY, fZ, fW};
    std::memcpy(packet.GetCursor(), afValues, sizeof(afValues));
    packet.Advance();
}

inline void PushVector(const Vector3 &vector) {
    PushFloats(vector.x, vector.y, vector.z, vector.w);
}

// A quadword of three VIF NOPs and one VIF code.
inline void PushVifCode(unsigned int nCode) {
    DmaPacket &packet = DmaPacket::shared();
    packet.GetCursor()->mWord[0] = 0;
    packet.GetCursor()->mWord[1] = 0;
    packet.GetCursor()->mWord[2] = 0;
    packet.GetCursor()->mWord[kVifCodeWord] = nCode;
    packet.Advance();
}

inline DmaQuadword *PushClearedQuadword() {
    DmaPacket &packet = DmaPacket::shared();
    DmaQuadword *pQuadword = packet.GetCursor();
    pQuadword->mDword[kLowDword] = 0;
    pQuadword->mDword[kHighDword] = 0;
    packet.Advance();
    return pQuadword;
}

inline unsigned int UnpackCode(int nAddress, int nQwords) {
    return static_cast<unsigned int>(nAddress) |
           (static_cast<unsigned int>(nQwords) << kVifUnpackNumShift) | kVifUnpackFlg |
           kVifUnpackV4;
}

inline unsigned int DmaAddressOf(const void *pAddress) {
    return static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pAddress));
}

} // namespace

PsMultiMesh::PsMultiMesh(const char *pszName) : RndObject(pszName), RndMultiMesh(pszName) {
}

int PsMultiMesh::DrawShowing() {
    if (mTransforms.empty()) {
        return 1;
    }
    RndMesh *pMesh = mMesh;
    if (pMesh == nullptr) {
        return 1;
    }
    if (static_cast<PsMesh *>(pMesh->GetGeomOwner())->GetNumFaces() == 0) {
        return 1;
    }

    int nPass = 0;
    RndMat *pMat = pMesh->GetMat();
    // The binary ends the range 15 bytes after the end node of the list. The end node has no
    // address in portable C++. The range ends after the last transform instead.
    SyncDCache(&mTransforms.front(), &mTransforms.back() + 1);
    do {
        static_cast<PsMesh *>(mMesh)->SetZTest(nPass);
        if (pMat != nullptr) {
            nPass = static_cast<PsMat *>(pMat)->Select(nPass);
        } else {
            PsMat::SelectDefault();
        }
        DrawInstances();
    } while (nPass != 0);
    return 1;
}

void PsMultiMesh::DrawInstances() {
    DmaPacket &frame = DmaPacket::shared();
    frame.CloseGifTag(1, 0);

    PushVifCode(UnpackCode(0, kConstantQwords));
    PushVector(g_viewProjectXfm.mBasisX);
    PushVector(g_viewProjectXfm.mBasisY);
    PushVector(g_viewProjectXfm.mBasisZ);
    PushVector(g_viewProjectXfm.mTranslation);
    PushFloats(g_colorScale.r, g_colorScale.g, g_colorScale.b, g_colorScale.a);
    PushFloats(g_viewportXfm.mBasisX.x, g_viewportXfm.mBasisY.y, g_viewportXfm.mBasisZ.z, 0.0f);
    PushFloats(g_viewportXfm.mTranslation.x,
               g_viewportXfm.mTranslation.y,
               g_viewportXfm.mTranslation.z,
               0.0f);
    PushVector(g_uvXfm.mBasisX);
    PushVector(g_uvXfm.mBasisY);
    PushVector(g_uvXfm.mTranslation);
    const unsigned long long qwPrim =
        kPrimTriangle | (static_cast<unsigned long long>(g_nSelectedFlat ^ 1) << kPrimIipShift) |
        (static_cast<unsigned long long>(g_nTextured) << kPrimTmeShift) |
        (static_cast<unsigned long long>(g_nAlphaBlendEnabled) << kPrimAbeShift);
    PushQuadword((qwPrim << kGifTagPrimShift) | kFaceGifTag, kFaceGifRegs);
    PsEnviron::SelectLights(nullptr, nullptr);

    MemHandle *pPacket;
    int nSetupQwords;
    int nVuEnd;
    static_cast<PsMesh *>(mMesh)->FacesPacket(pPacket, nSetupQwords, nVuEnd);
    auto *pData = static_cast<DmaQuadword *>(pPacket->Lock());
    if (mMesh->GetGeomOwner()->GetMutable() == 0) {
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        frame.OpenDmaTag();
        frame.CloseDmaTag(kDmaTagIdRef, DmaAddressOf(pData), nSetupQwords);
        frame.OpenDmaTag();
    } else {
        (void)ThePs.PacketQwordsFree(); // Yes, the binary discards this call's result.
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        DmaQuadword *pDest = frame.GetCursor();
        DmaPacket packet;
        packet.SetBase(nullptr); // Yes, the binary clears the base that Init() then replaces.
        packet.Init(pData, kDmaChannelToSpr, 1);
        packet.SetCursor(pData + nSetupQwords);
        packet.Send(DmaPacket::kModeNormal, DmaAddressOf(pDest));
        frame.SetCursor(pDest + nSetupQwords);
        frame.OpenDmaTag();
    }
    pPacket->Unlock();

    const int nRoom =
        kVuDataEnd - nVuEnd < kMaxUnpackQwords ? kVuDataEnd - nVuEnd : kMaxUnpackQwords;
    const int nPerBatch = nRoom / kTransformQwords;
    bool bFirst = true;
    auto it = mTransforms.begin();
    while (it != mTransforms.end()) {
        DmaQuadword *pUnpack = PushClearedQuadword();
        DmaQuadword *pCount = PushClearedQuadword();
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        int nCount = 0;
        if (nPerBatch > 0) {
            while (it != mTransforms.end()) {
                ++nCount;
                PushQuadword(
                    (static_cast<unsigned long long>(DmaAddressOf(&*it)) << kDmaTagAddressShift) |
                        kDmaRefTransformTag,
                    0);
                ++it;
                if (nCount >= nPerBatch) {
                    break;
                }
            }
        }
        frame.OpenDmaTag();
        pUnpack->mWord[kVifCodeWord] = UnpackCode(nVuEnd, nCount * kTransformQwords + 1);
        pCount->mWord[0] = static_cast<unsigned int>(nCount);

        PushVifCode(bFirst ? kVifMscalInstanced : kVifMscnt);
        bFirst = false;
        ThePs.Flush();
    }
}
