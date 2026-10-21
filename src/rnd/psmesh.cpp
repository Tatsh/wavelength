#include "rnd/psmesh.h"

#include <cstdint>
#include <cstring>
#include <vector>

#include <eekernel.h>

#include "math/sphere.h"
#include "math/transformops.h"
#include "rnd/dmapacket.h"
#include "rnd/ps.h"
#include "rnd/pscam.h"
#include "rnd/psenviron.h"
#include "rnd/psmat.h"
#include "rnd/rndmanager.h"

namespace {

constexpr int kNoHeap = -1;
constexpr int kQwordBytes = 16;
constexpr int kPacketBytes = 0x30000;

// VIF codes. An UNPACK code has its count at bit 16 and adds the TOPS register to its address.
constexpr unsigned int kVifStcycl = 0x01000000;
constexpr unsigned int kVifMscal = 0x14000000;
constexpr unsigned int kVifMscnt = 0x17000000;
constexpr unsigned int kVifUnpackV2_32 = 0x64000000;
constexpr unsigned int kVifUnpackV2_16 = 0x65000000;
constexpr unsigned int kVifUnpackV3_32 = 0x68000000;
constexpr unsigned int kVifUnpackV3_16 = 0x69000000;
constexpr unsigned int kVifUnpackV4_32 = 0x6c000000;
constexpr int kVifNumShift = 16;
constexpr unsigned int kVifNumMask = 0xff;
constexpr unsigned int kVifUnpackTops = 0x8000;
constexpr int kStcyclWlShift = 8;

// STCYCL settings, write cycle and cycle length.
constexpr unsigned int kStcycl1Of1 = kVifStcycl | (1 << kStcyclWlShift) | 1;
constexpr unsigned int kStcycl1Of2 = kVifStcycl | (1 << kStcyclWlShift) | 2;
constexpr unsigned int kStcycl1Of4 = kVifStcycl | (1 << kStcyclWlShift) | 4;
constexpr unsigned int kStcycl2Of4 = kVifStcycl | (2 << kStcyclWlShift) | 4;

// DMA tags of a mutable mesh, CNT for the VIF codes and REF for each vertex.
constexpr unsigned long long kDmaTagCnt = 0x10000000;
constexpr unsigned long long kDmaTagRef = 0x30000000;
constexpr int kDmaTagAddressShift = 32;
constexpr int kDmaTagIdCnt = 1;
constexpr int kDmaTagIdRef = 3;

// The face microprogram. Each vertex occupies four quadwords from kFaceVuVerts, the batch settings
// are at kFaceVuSettings, and the indices follow the vertices, one V3-16 vector for each face.
// A batch takes vertices while the budget lasts, at a cost of 4 a vertex and 1 a face.
constexpr unsigned int kFaceProgram = 0x63e;
constexpr int kFaceVuSettings = 0x15;
constexpr int kFaceVuVerts = 0x16;
constexpr int kFaceVertQwords = 4;
constexpr int kFaceVertCost = 4;
constexpr int kFaceBatchBudget = 0x12b;
constexpr int kFaceIndices = 3;
constexpr int kFaceUnpackVerts = 64;
constexpr int kFaceColorOffset = 2;
constexpr int kFaceTexOffset = 3;

// The edge microprogram. Each vertex is a position and a colour slot from kEdgeVuVerts, the batch
// settings are at kEdgeVuSettings, and the indices follow, one V2-16 vector for each edge.
constexpr unsigned int kEdgeProgram = 0x5a8;
constexpr int kEdgeVuSettings = 8;
constexpr int kEdgeVuVerts = 9;
constexpr int kEdgeVertQwords = 2;
constexpr int kEdgeVertCost = 2;
constexpr int kEdgeBatchBudget = 0x140;
constexpr int kEdgeIndices = 2;

// A vertex of RndMesh is four quadwords, and a REF tag sends the whole vertex or its position.
constexpr unsigned long long kVertQwords = 4;
constexpr unsigned long long kVertPositionQwords = 1;

// The settings the draw routines unpack ahead of the lighting block, 21 quadwords for faces and 8
// for edges, from VU1 address 0.
constexpr unsigned int kFaceSettingsQwords = 0x15;
constexpr unsigned int kEdgeSettingsQwords = 8;

// GIF tags the microprograms send with each primitive, PRE and EOP set, PRIM at bit 47.
constexpr unsigned long long kGifTagTriangles = 0x9000400000008000ULL;
constexpr unsigned long long kGifRegsTriangles = 0x412412412ULL;
constexpr unsigned long long kGifTagVertex = 0x3000400000008000ULL;
constexpr unsigned long long kGifRegsVertex = 0x412;
constexpr unsigned long long kGifRegsLine = 0x441;
constexpr int kGifTagPrimShift = 47;
constexpr unsigned int kPrimLine = 1;
constexpr unsigned int kPrimTriangle = 3;
constexpr unsigned int kPrimTriangleFan = 5;
constexpr int kPrimIipShift = 3;
constexpr int kPrimTmeShift = 4;
constexpr int kPrimFgeShift = 5;
constexpr int kPrimAbeShift = 6;
constexpr unsigned int kPrimAbe = 1 << kPrimAbeShift;
constexpr unsigned int kPrimAa1 = 0x80;

// The depth registers of SetZTest().
constexpr int kGsRegTest1 = 0x47;
constexpr int kGsRegZbuf1 = 0x4e;
constexpr unsigned long long kZbufZmskMask = 1ULL << 32;
constexpr int kZbufZmskShift = 32;
constexpr int kTestZtstShift = 17;
constexpr unsigned long long kTestZtstMask = 0x60000;
constexpr unsigned long long kZtstNever = 0;
constexpr unsigned long long kZtstAlways = 1;
constexpr unsigned long long kZtstGequal = 2;
constexpr unsigned long long kZtstGreater = 3;
constexpr int kPassFaces = 0;
constexpr int kPassEdges = 1;
constexpr int kPassCount = 2;

constexpr float kColorScale = 255.0f;
constexpr float kAlphaScale = 128.0f;

// The VU0 microprograms that skin one vertex with one bone or with two.
constexpr int kLowDword = 0;
constexpr int kHighDword = 1;
constexpr int kWordX = 0;
constexpr int kWordY = 1;
constexpr int kWordZ = 2;
constexpr int kWordW = 3;

// Writes halfwords one after another into quadwords.
class HalfwordWriter {
public:
    explicit HalfwordWriter(DmaQuadword *pQuad) : mQuad(pQuad), mCount(0) {
    }

    void Put(int nValue) {
        mQuad[mCount / kHalves].mHalf[mCount % kHalves] = static_cast<unsigned short>(nValue);
        ++mCount;
    }

    int Count() const {
        return mCount;
    }

    // Pad with zero to a whole quadword and report the quadword after.
    DmaQuadword *Align() {
        while (mCount % kHalves != 0) {
            Put(0);
        }
        return mQuad + mCount / kHalves;
    }

private:
    static constexpr int kHalves = 8;

    DmaQuadword *mQuad;
    int mCount;
};

// Writes words and floats one after another into quadwords.
class WordWriter {
public:
    WordWriter(DmaQuadword *pQuad, int nFirst) : mQuad(pQuad), mCount(nFirst) {
    }

    void Put(unsigned int nValue) {
        mQuad[mCount / kWords].mWord[mCount % kWords] = nValue;
        ++mCount;
    }

    void Put(float fValue) {
        mQuad[mCount / kWords].mFloat[mCount % kWords] = fValue;
        ++mCount;
    }

    // Pad with zero to a whole quadword and report the quadword after.
    DmaQuadword *Align() {
        while (mCount % kWords != 0) {
            Put(0U);
        }
        return mQuad + mCount / kWords;
    }

private:
    static constexpr int kWords = 4;

    DmaQuadword *mQuad;
    int mCount;
};

inline unsigned int DmaAddressOf(const void *pAddress) {
    return static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pAddress));
}

// The last byte before an end, the end SyncDCache() takes.
inline void *ByteBefore(void *pEnd) {
    return static_cast<unsigned char *>(pEnd) - 1;
}

inline unsigned int VifUnpack(unsigned int nCode, unsigned int nCount, int nAddress) {
    return nCode | (nCount << kVifNumShift) | kVifUnpackTops | static_cast<unsigned int>(nAddress);
}

// The heap the packets come from, rnd, looked up once for each call site. A missing heap is 0.
inline int RndHeap(int &nCached) {
    if (nCached == kNoHeap) {
        nCached = MemFindHeap("rnd");
        if (nCached == kNoHeap) {
            return 0;
        }
    }
    return nCached;
}

inline void PushVector(const Vector3 &v) {
    DmaQuadword &quad = *DmaPacket::shared().mCursor;
    quad.mFloat[kWordX] = v.x;
    quad.mFloat[kWordY] = v.y;
    quad.mFloat[kWordZ] = v.z;
    quad.mFloat[kWordW] = v.w;
    ++DmaPacket::shared().mCursor;
}

inline void PushFloats(float fX, float fY, float fZ, float fW) {
    DmaQuadword &quad = *DmaPacket::shared().mCursor;
    quad.mFloat[kWordX] = fX;
    quad.mFloat[kWordY] = fY;
    quad.mFloat[kWordZ] = fZ;
    quad.mFloat[kWordW] = fW;
    ++DmaPacket::shared().mCursor;
}

inline void PushDwords(unsigned long long qwLow, unsigned long long qwHigh) {
    DmaQuadword &quad = *DmaPacket::shared().mCursor;
    quad.mDword[kLowDword] = qwLow;
    quad.mDword[kHighDword] = qwHigh;
    ++DmaPacket::shared().mCursor;
}

inline void PushTransform(const Transform &xfm) {
    PushVector(xfm.mBasisX);
    PushVector(xfm.mBasisY);
    PushVector(xfm.mBasisZ);
    PushVector(xfm.mTranslation);
}

// Open the settings of a microprogram, an UNPACK of a count of quadwords to VU1 address 0.
inline void PushSettingsUnpack(unsigned int nQwords) {
    DmaPacket &packet = DmaPacket::shared();
    packet.CloseGifTag(1, 0);
    packet.mCursor->mWord[kWordX] = 0;
    packet.mCursor->mWord[kWordY] = 0;
    packet.mCursor->mWord[kWordZ] = 0;
    packet.mCursor->mWord[kWordW] = VifUnpack(kVifUnpackV4_32, nQwords, 0);
    ++packet.mCursor;
}

// Send a packet of a geometry. A mesh that is not mutable sends it by reference. A mutable mesh
// copies it into the frame by DMA, for its REF tags are only valid in a DMA chain run from main
// memory.
inline void SendPacket(const PsMesh &geom, DmaQuadword *pData, int nQwords) {
    DmaPacket &frame = DmaPacket::shared();
    if (geom.mMutable == 0) {
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        frame.OpenDmaTag();
        frame.CloseDmaTag(kDmaTagIdRef, DmaAddressOf(pData), nQwords);
        frame.OpenDmaTag();
        return;
    }
    (void)ThePs.PacketQwordsFree(); // Yes, the binary discards this call's result.
    frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
    DmaQuadword *pDest = frame.mCursor;
    DmaPacket copy;
    copy.mBase = nullptr; // Yes, the binary clears the base that Init() then replaces.
    copy.Init(pData, DMAC_TO_SPR, 1);
    copy.mCursor = pData + nQwords;
    copy.Send(DmaPacket::kModeNormal, DmaAddressOf(pDest));
    frame.mCursor = pDest + nQwords;
    ThePs.Flush();
}

} // namespace

alignas(16) Transform g_identityXfm;
unsigned short g_vertexCache[PsMesh::kVertexCacheSize];

PsMesh::PsMesh(const char *pszName)
    : RndObject(pszName), RndMesh(pszName), mSkinVerts(nullptr), mFacesPacket(nullptr),
      mEdgesPacket(nullptr) {
}

PsMesh::~PsMesh() {
    ThePs.ReleaseAfterFrame(mFacesPacket);
    ThePs.ReleaseAfterFrame(mEdgesPacket);
}

void PsMesh::Copy(const RndObject *pSource, int nFlags) {
    RndMesh::Copy(pSource, nFlags);
    // A source that is not a PsMesh is not handled.
    const PsMesh *pMesh = pSource != nullptr ? dynamic_cast<const PsMesh *>(pSource) : nullptr;

    if (mFacesPacket == nullptr && pMesh->mFacesPacket != nullptr) {
        mFacesSetupQwords = pMesh->mFacesSetupQwords;
        mFacesVuEnd = pMesh->mFacesVuEnd;
        mFacesQwords = pMesh->mFacesQwords;
        const int nBytes = mFacesQwords * kQwordBytes;
        // NTSC-U/C: 0x003b07c0, PAL: 0x0042f658
        static int sHeap = kNoHeap;
        MemPushHeap(RndHeap(sHeap));
        mFacesPacket = MemHandle::Alloc(nBytes, "mFacesPacket");
        MemPopHeap();
        const void *pFrom = pMesh->mFacesPacket->Lock();
        std::memcpy(mFacesPacket->Lock(), pFrom, nBytes);
        pMesh->mFacesPacket->Unlock();
        mFacesPacket->Unlock();
    }

    if (mEdgesPacket == nullptr && pMesh->mEdgesPacket != nullptr) {
        mEdgesQwords = pMesh->mEdgesQwords;
        // NTSC-U/C: 0x003b07c4, PAL: 0x0042f65c
        static int sHeap = kNoHeap;
        MemPushHeap(RndHeap(sHeap));
        mEdgesPacket = MemHandle::Alloc(mEdgesQwords * kQwordBytes, "mEdgesPacket");
        MemPopHeap();
        const void *pFrom = pMesh->mEdgesPacket->Lock();
        std::memcpy(mEdgesPacket->Lock(), pFrom, mEdgesQwords * kQwordBytes);
        mEdgesPacket->Unlock();
        pMesh->mEdgesPacket->Unlock();
    }
}

void PsMesh::BuildFacesPacket() {
    ThePs.ReleaseAfterFrame(mFacesPacket);
    mFacesPacket = nullptr;
    if (mFaces.empty() || mVerts.empty()) {
        return;
    }

    // NTSC-U/C: 0x003b07c8, PAL: 0x0042f660
    static int sHeap = kNoHeap;
    MemPushHeap(RndHeap(sHeap));
    mFacesPacket = MemHandle::Alloc(kPacketBytes, "mFacesPacket");
    MemPopHeap();

    auto *pBase = static_cast<DmaQuadword *>(mFacesPacket->Lock());
    DmaQuadword *pTag = nullptr;
    DmaQuadword *pCursor = pBase;
    if (mMutable != 0) {
        pTag = pBase;
        pCursor = pBase + 1;
    }

    bool bFirst = true;
    auto face = mFaces.begin();
    while (face != mFaces.end()) {
        DmaQuadword *pHeader = pCursor;
        *pCursor = DmaQuadword{};
        ++pCursor;
        int nBudget = kFaceBatchBudget;
        int nCached = 0;
        HalfwordWriter indices(pCursor);
        while (face != mFaces.end()) {
            for (int k = 0; k < kFaceIndices; ++k) {
                const unsigned short nVert = face->mVerts[k];
                int i = nCached - 1;
                while (i >= 0 && g_vertexCache[i] != nVert) {
                    --i;
                }
                if (i >= 0) {
                    indices.Put(i * kFaceVertQwords);
                } else {
                    nBudget -= kFaceVertCost;
                    g_vertexCache[nCached] = nVert;
                    indices.Put(nCached * kFaceVertQwords);
                    ++nCached;
                }
            }
            ++face;
            if (--nBudget < 0) {
                break;
            }
        }

        const int nFaces = indices.Count() / kFaceIndices;
        pHeader->mWord[kWordW] =
            VifUnpack(kVifUnpackV3_16, nFaces, nCached * kFaceVertQwords + kFaceVuVerts);
        pCursor = indices.Align();

        pCursor->mDword[kLowDword] = 0;
        pCursor->mWord[kWordZ] = 0;
        pCursor->mWord[kWordW] = VifUnpack(kVifUnpackV4_32, 1, kFaceVuSettings);
        ++pCursor;
        pCursor->mWord[kWordW] = nFaces + nCached * kFaceVertQwords;
        pCursor->mWord[kWordZ] = 1;
        pCursor->mWord[kWordX] = nCached;
        pCursor->mWord[kWordY] = nFaces;
        mFacesVuEnd = nFaces + kFaceVuVerts + nCached * kFaceVertQwords;
        ++pCursor;

        int nVuAddress = kFaceVuVerts;
        int nRemaining = nCached;
        const unsigned short *pIndex = g_vertexCache;
        do {
            const int nChunk = nRemaining > kFaceUnpackVerts ? kFaceUnpackVerts : nRemaining;
            nRemaining -= nChunk;
            const unsigned short *pEnd = pIndex + nChunk;
            if (mMutable != 0) {
                pCursor->mDword[kLowDword] = 0;
                pCursor->mWord[kWordZ] = 0;
                pCursor->mWord[kWordW] = VifUnpack(
                    kVifUnpackV4_32, (nChunk * kFaceVertQwords) & kVifNumMask, nVuAddress);
                ++pCursor;
                pTag->mDword[kLowDword] =
                    kDmaTagCnt | static_cast<unsigned long long>(pCursor - pTag - 1);
                for (; pIndex != pEnd; ++pIndex) {
                    pCursor->mDword[kLowDword] =
                        (static_cast<unsigned long long>(DmaAddressOf(&mVerts[*pIndex]))
                         << kDmaTagAddressShift) |
                        kDmaTagRef | kVertQwords;
                    ++pCursor;
                }
                pTag = pCursor;
                ++pCursor;
            } else {
                WordWriter words(pCursor, 0);
                words.Put(kStcycl2Of4);
                words.Put(VifUnpack(kVifUnpackV3_32, nChunk * 2, nVuAddress));
                for (const unsigned short *p = pIndex; p != pEnd; ++p) {
                    const Vert &vert = mVerts[*p];
                    words.Put(vert.mPos.x);
                    words.Put(vert.mPos.y);
                    words.Put(vert.mPos.z);
                    words.Put(vert.mNorm.x);
                    words.Put(vert.mNorm.y);
                    words.Put(vert.mNorm.z);
                }
                words.Put(kStcycl1Of4);
                words.Put(VifUnpack(kVifUnpackV4_32, nChunk, nVuAddress + kFaceColorOffset));
                for (const unsigned short *p = pIndex; p != pEnd; ++p) {
                    const Vert &vert = mVerts[*p];
                    words.Put(vert.mColor.r);
                    words.Put(vert.mColor.g);
                    words.Put(vert.mColor.b);
                    words.Put(vert.mColor.a);
                }
                words.Put(VifUnpack(kVifUnpackV2_32, nChunk, nVuAddress + kFaceTexOffset));
                for (; pIndex != pEnd; ++pIndex) {
                    const Vert &vert = mVerts[*pIndex];
                    words.Put(vert.mTex.x);
                    words.Put(vert.mTex.y);
                }
                words.Put(kStcycl1Of1);
                pCursor = words.Align();
            }
            nVuAddress += nChunk * kFaceVertQwords;
        } while (nRemaining != 0);

        if (bFirst) {
            bFirst = false;
            pCursor->mWord[kWordX] = kVifMscal | kFaceProgram;
            mFacesSetupQwords = static_cast<unsigned short>(pCursor - pBase - mMutable);
        } else {
            pCursor->mWord[kWordX] = kVifMscnt;
        }
        pCursor->mWord[kWordY] = 0;
        pCursor->mDword[kHighDword] = 0;
        ++pCursor;
    }
    if (mMutable != 0) {
        pTag->mDword[kLowDword] = kDmaTagCnt | static_cast<unsigned long long>(pCursor - pTag - 1);
    }

    mFacesQwords = static_cast<unsigned short>(pCursor - pBase);
    const int nBytes = mFacesQwords * kQwordBytes;
    mFacesPacket->Resize(nBytes);
    SyncDCache(pBase, ByteBefore(pBase + mFacesQwords));
    mFacesPacket->Unlock();
}

void PsMesh::BuildEdgesPacket() {
    ThePs.ReleaseAfterFrame(mEdgesPacket);
    mEdgesPacket = nullptr;
    if (mEdges.empty() || mVerts.empty()) {
        return;
    }

    // NTSC-U/C: 0x003b07cc, PAL: 0x0042f664
    static int sHeap = kNoHeap;
    MemPushHeap(RndHeap(sHeap));
    mEdgesPacket = MemHandle::Alloc(kPacketBytes, "mEdgesPacket");
    MemPopHeap();

    auto *pBase = static_cast<DmaQuadword *>(mEdgesPacket->Lock());
    DmaQuadword *pTag = nullptr;
    DmaQuadword *pCursor = pBase;
    if (mMutable != 0) {
        pTag = pBase;
        pCursor = pBase + 1;
    }

    bool bFirst = true;
    auto edge = mEdges.begin();
    while (edge != mEdges.end()) {
        DmaQuadword *pHeader = pCursor;
        *pCursor = DmaQuadword{};
        ++pCursor;
        int nBudget = kEdgeBatchBudget;
        int nCached = 0;
        HalfwordWriter indices(pCursor);
        while (edge != mEdges.end()) {
            for (int k = 0; k < kEdgeIndices; ++k) {
                const unsigned short nVert = edge->mVerts[k];
                int i = nCached - 1;
                while (i >= 0 && g_vertexCache[i] != nVert) {
                    --i;
                }
                if (i >= 0) {
                    indices.Put(i * kEdgeVertQwords);
                } else {
                    g_vertexCache[nCached] = nVert;
                    indices.Put(nCached * kEdgeVertQwords);
                    ++nCached;
                    nBudget -= kEdgeVertCost;
                }
            }
            ++edge;
            if (--nBudget < 0) {
                break;
            }
        }

        const int nEdges = indices.Count() / kEdgeIndices;
        pHeader->mWord[kWordW] =
            VifUnpack(kVifUnpackV2_16, nEdges, nCached * kEdgeVertQwords + kEdgeVuVerts);
        pCursor = indices.Align();

        pCursor->mDword[kLowDword] = 0;
        pCursor->mWord[kWordZ] = 0;
        pCursor->mWord[kWordW] = VifUnpack(kVifUnpackV4_32, 1, kEdgeVuSettings);
        ++pCursor;
        pCursor->mWord[kWordX] = nCached;
        pCursor->mWord[kWordY] = nEdges;
        pCursor->mDword[kHighDword] = 0;
        ++pCursor;
        pCursor->mWord[kWordZ] = kStcycl1Of2;
        pCursor->mDword[kLowDword] = 0;

        const unsigned short *pIndex = g_vertexCache;
        const unsigned short *pEnd = g_vertexCache + nCached;
        if (mMutable != 0) {
            pCursor->mWord[kWordW] = VifUnpack(kVifUnpackV4_32, nCached, kEdgeVuVerts);
            ++pCursor;
            pTag->mDword[kLowDword] =
                kDmaTagCnt | static_cast<unsigned long long>(pCursor - pTag - 1);
            for (; pIndex != pEnd; ++pIndex) {
                pCursor->mDword[kLowDword] =
                    (static_cast<unsigned long long>(DmaAddressOf(&mVerts[*pIndex]))
                     << kDmaTagAddressShift) |
                    kDmaTagRef | kVertPositionQwords;
                ++pCursor;
            }
            pTag = pCursor;
            ++pCursor;
        } else {
            pCursor->mWord[kWordW] = VifUnpack(kVifUnpackV3_32, nCached, kEdgeVuVerts);
            WordWriter words(pCursor + 1, 0);
            for (; pIndex != pEnd; ++pIndex) {
                const Vert &vert = mVerts[*pIndex];
                words.Put(vert.mPos.x);
                words.Put(vert.mPos.y);
                words.Put(vert.mPos.z);
            }
            pCursor = words.Align();
        }

        if (bFirst) {
            bFirst = false;
            pCursor->mWord[kWordX] = kVifMscal | kEdgeProgram;
        } else {
            pCursor->mWord[kWordX] = kVifMscnt;
        }
        pCursor->mDword[kHighDword] = 0;
        pCursor->mWord[kWordY] = kStcycl1Of1;
        ++pCursor;
    }
    if (mMutable != 0) {
        pTag->mDword[kLowDword] = kDmaTagCnt | static_cast<unsigned long long>(pCursor - pTag - 1);
    }

    mEdgesQwords = static_cast<unsigned short>(pCursor - pBase);
    const int nBytes = mEdgesQwords * kQwordBytes;
    mEdgesPacket->Resize(nBytes);
    SyncDCache(pBase, ByteBefore(pBase + mEdgesQwords));
    mEdgesPacket->Unlock();
}

void PsMesh::Sync(int nFlags) {
    if (mGeomOwner != this) {
        return;
    }
    const int nEdges = nFlags & kSyncEdges;
    const int nFaces = nFlags & kSyncFaces;

    if ((nFlags & kSyncVertexData) != 0) {
        mSkinVerts.reset();
        if (mBones != nullptr) {
            mMutable = 1;
            mSkinVerts = std::make_unique<SkinVert[]>(mVerts.size());
            SkinVert *pSkin = mSkinVerts.get();
            for (const auto &vert : mVerts) {
                pSkin->mPos[kWordX] = vert.mPos.x;
                pSkin->mPos[kWordY] = vert.mPos.y;
                pSkin->mPos[kWordZ] = vert.mPos.z;
                pSkin->mWeight1 = vert.mWeight1;
                pSkin->mNorm[kWordX] = vert.mNorm.x;
                pSkin->mNorm[kWordY] = vert.mNorm.y;
                pSkin->mNorm[kWordZ] = vert.mNorm.z;
                pSkin->mWeight2 = vert.mWeight2;
                ++pSkin;
            }
            FlushCache(WRITEBACK_DCACHE);
        }
    }

    if (nFaces != 0) {
        BuildFacesPacket();
        mNumFaces = static_cast<unsigned short>(mFaces.size());
        std::vector<Face>().swap(mFaces);
    }
    if (nEdges != 0) {
        BuildEdgesPacket();
        mNumEdges = static_cast<unsigned short>(mEdges.size());
        std::vector<Edge>().swap(mEdges);
    }
    if (mMutable == 0) {
        std::vector<Vert>().swap(mVerts);
    } else {
        SyncDCache(mVerts.data(), ByteBefore(mVerts.data() + mVerts.size()));
    }
}

void PsMesh::SetZTest(int nPass) {
    if (nPass >= kPassCount) {
        return;
    }
    bool bNoWrite = false;
    if (nPass != kPassFaces || mZMode == kZModeDisable || mZMode == kZModeZReadOnly ||
        mZMode == kZModeWReadOnly) {
        bNoWrite = true;
    }

    unsigned long long qwTest;
    if (mZMode == kZModeDisable || mZFunc == kZFuncAlways) {
        qwTest = kZtstAlways;
    } else if (nPass != kPassFaces) {
        qwTest = kZtstGequal;
    } else if (mZFunc == kZFuncLessEqual || mZFunc == kZFuncEqual) {
        qwTest = kZtstGequal;
    } else {
        qwTest = mZFunc == kZFuncLess ? kZtstGreater : kZtstNever;
    }
    ThePs.SetGsReg(
        kGsRegZbuf1, static_cast<unsigned long long>(bNoWrite) << kZbufZmskShift, kZbufZmskMask);
    ThePs.SetGsReg(kGsRegTest1, qwTest << kTestZtstShift, kTestZtstMask);
}

void PsMesh::DrawFaces(const Transform &xfm) {
    PushSettingsUnpack(kFaceSettingsQwords);
    PushVector(g_invGuardBandScale);
    Transform viewProject;
    sceVu0MulAffineMatrix(
        &viewProject.mBasisX.x, &g_viewProjectUnscaledXfm.mBasisX.x, &xfm.mBasisX.x);
    PushTransform(viewProject);
    PushFloats(g_colorScale.r, g_colorScale.g, g_colorScale.b, g_colorScale.a);
    PushFloats(g_viewportUnscaledXfm.mBasisX.x,
               g_viewportUnscaledXfm.mBasisY.y,
               g_viewportUnscaledXfm.mBasisZ.z,
               g_fogScale);
    PushFloats(g_viewportUnscaledXfm.mTranslation.x,
               g_viewportUnscaledXfm.mTranslation.y,
               g_viewportUnscaledXfm.mTranslation.z,
               g_fogOffset);
    PushVector(g_uvXfm.mBasisX);
    PushVector(g_uvXfm.mBasisY);
    PushVector(g_uvXfm.mTranslation);

    const unsigned long long qwIip =
        static_cast<unsigned long long>(static_cast<unsigned int>(g_nSelectedFlat ^ 1))
        << kPrimIipShift;
    const unsigned long long qwTme =
        static_cast<unsigned long long>(static_cast<unsigned int>(g_nTextured)) << kPrimTmeShift;
    const unsigned long long qwFge =
        static_cast<unsigned long long>(static_cast<unsigned int>(g_nFogEnabled)) << kPrimFgeShift;
    const unsigned long long qwAbe =
        static_cast<unsigned long long>(static_cast<unsigned int>(g_nAlphaBlendEnabled))
        << kPrimAbeShift;
    const unsigned long long qwTriangles = qwTme | kPrimTriangle | qwIip | qwFge | qwAbe;
    const unsigned long long qwFan = qwIip | qwTme | kPrimTriangleFan | qwFge | qwAbe;
    PushDwords((qwTriangles << kGifTagPrimShift) | kGifTagTriangles, kGifRegsTriangles);
    PushDwords((qwFan << kGifTagPrimShift) | kGifTagVertex, kGifRegsVertex);

    PsEnviron::SelectLights(&xfm, &mSphere);

    auto *pGeom = static_cast<PsMesh *>(mGeomOwner);
    auto *pData = static_cast<DmaQuadword *>(pGeom->mFacesPacket->Lock());
    SendPacket(*pGeom, pData, pGeom->mFacesQwords);
    pGeom->mFacesPacket->Unlock();
}

void PsMesh::DrawEdges(const Transform &xfm) {
    PushSettingsUnpack(kEdgeSettingsQwords);
    Transform viewProject;
    sceVu0MulAffineMatrix(
        &viewProject.mBasisX.x, &g_viewProjectBiasedXfm.mBasisX.x, &xfm.mBasisX.x);
    PushTransform(viewProject);
    const unsigned long long qwLine =
        (static_cast<unsigned long long>(static_cast<unsigned int>(g_nFogEnabled))
         << kPrimFgeShift) |
        kPrimAa1 | kPrimAbe | kPrimLine;
    PushDwords((qwLine << kGifTagPrimShift) | kGifTagVertex, kGifRegsLine);
    PushFloats(
        g_viewportXfm.mBasisX.x, g_viewportXfm.mBasisY.y, g_viewportXfm.mBasisZ.z, g_fogScale);
    PushFloats(g_viewportXfm.mTranslation.x,
               g_viewportXfm.mTranslation.y,
               g_viewportXfm.mTranslation.z,
               g_fogOffset);

    const Color color = mMat != nullptr ? mMat->mEdgeColor : Color{1.0f, 1.0f, 1.0f, 1.0f};
    const auto nRed = static_cast<unsigned int>(color.r * kColorScale);
    const auto nGreen = static_cast<unsigned int>(color.g * kColorScale);
    const auto nBlue = static_cast<unsigned int>(color.b * kColorScale);
    const auto nAlpha = static_cast<unsigned int>(color.a * kAlphaScale);
    DmaQuadword &quad = *DmaPacket::shared().mCursor;
    quad.mWord[kWordX] = nRed;
    quad.mWord[kWordY] = nGreen;
    quad.mWord[kWordZ] = nBlue;
    quad.mWord[kWordW] = nAlpha;
    ++DmaPacket::shared().mCursor;

    auto *pGeom = static_cast<PsMesh *>(mGeomOwner);
    auto *pData = static_cast<DmaQuadword *>(pGeom->mEdgesPacket->Lock());
    if (pGeom->mMutable == 0) {
        DmaPacket &frame = DmaPacket::shared();
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        frame.OpenDmaTag();
        frame.CloseDmaTag(kDmaTagIdRef, DmaAddressOf(pData), pGeom->mEdgesQwords);
        frame.OpenDmaTag();
    } else {
        (void)ThePs.PacketQwordsFree(); // Yes, the binary discards this call's result.
        DmaPacket &frame = DmaPacket::shared();
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        for (int n = pGeom->mEdgesQwords; n-- != 0;) {
            *frame.mCursor = *pData;
            ++frame.mCursor;
            ++pData;
        }
        ThePs.Flush();
    }
    pGeom->mEdgesPacket->Unlock();
}

const Transform &PsMesh::SkinXfm() {
    if (mBones == nullptr) {
        return mTransOwner->BillboardXfm();
    }

    Transform first;
    Multiply(first, mBones->mTrans[0]->mWorldXfm, mBones->mXfms[0]);
    const Transform &world = mTransOwner->mWorldXfm;
    const SkinVert *pSkin = mSkinVerts.get();
    Vert *pOut = mVerts.data();
    const SkinVert *pEnd = mSkinVerts.get() + mVerts.size();
    if (mBones->mTrans[1] != nullptr) {
        Transform second;
        Multiply(second, mBones->mTrans[1]->mWorldXfm, mBones->mXfms[1]);
        // The VU0 microprogram at instruction 0x156, byte 0xab0, blends the two bones. Each run
        // takes the next bind pose while the results of the run before are read.
        __asm__ volatile(
            ".set noreorder\n"
            "lqc2 $vf18, 0x0(%[second])\n"
            "lqc2 $vf19, 0x10(%[second])\n"
            "lqc2 $vf20, 0x20(%[second])\n"
            "lqc2 $vf21, 0x30(%[second])\n"
            "lqc2 $vf3, 0x0(%[world])\n"
            "lqc2 $vf4, 0x10(%[world])\n"
            "lqc2 $vf5, 0x20(%[world])\n"
            "lqc2 $vf6, 0x30(%[world])\n"
            "lqc2 $vf7, 0x0(%[first])\n"
            "lqc2 $vf8, 0x10(%[first])\n"
            "lqc2 $vf9, 0x20(%[first])\n"
            "lqc2 $vf10, 0x30(%[first])\n"
            "lq $2, 0x0(%[skin])\n"
            "lq $3, 0x10(%[skin])\n"
            "qmtc2 $2, $vf11\n"
            "qmtc2 $3, $vf15\n"
            "vcallms 0xab0\n"
            "addi %[skin], %[skin], 0x20\n"
            "lq $2, 0x0(%[skin])\n"
            "lq $3, 0x10(%[skin])\n"
            "1:\n"
            "qmtc2.i $2, $vf11\n"
            "qmtc2 $3, $vf15\n"
            "vcallms 0xab0\n"
            "qmfc2 $2, $vf16\n"
            "qmfc2 $3, $vf12\n"
            "sq $2, 0x10(%[out])\n"
            "sq $3, 0x0(%[out])\n"
            "addi %[out], %[out], 0x40\n"
            "addi %[skin], %[skin], 0x20\n"
            "lq $2, 0x0(%[skin])\n"
            "lq $3, 0x10(%[skin])\n"
            "bne %[skin], %[end], 1b\n"
            "nop\n"
            "nop\n"
            "qmfc2.i $2, $vf16\n"
            "qmfc2 $3, $vf12\n"
            "sq $2, 0x10(%[out])\n"
            "sq $3, 0x0(%[out])\n"
            ".set reorder\n"
            : [skin] "+r"(pSkin), [out] "+r"(pOut)
            : [first] "r"(&first), [second] "r"(&second), [world] "r"(&world), [end] "r"(pEnd)
            : "$2", "$3", "memory");
    } else {
        // The VU0 microprogram at instruction 0x1ba, byte 0xdd0, moves each vertex by the one
        // bone, in the same pipeline.
        __asm__ volatile(".set noreorder\n"
                         "lqc2 $vf3, 0x0(%[world])\n"
                         "lqc2 $vf4, 0x10(%[world])\n"
                         "lqc2 $vf5, 0x20(%[world])\n"
                         "lqc2 $vf6, 0x30(%[world])\n"
                         "lqc2 $vf7, 0x0(%[first])\n"
                         "lqc2 $vf8, 0x10(%[first])\n"
                         "lqc2 $vf9, 0x20(%[first])\n"
                         "lqc2 $vf10, 0x30(%[first])\n"
                         "lq $2, 0x0(%[skin])\n"
                         "lq $3, 0x10(%[skin])\n"
                         "qmtc2 $2, $vf11\n"
                         "qmtc2 $3, $vf15\n"
                         "vcallms 0xdd0\n"
                         "addi %[skin], %[skin], 0x20\n"
                         "lq $2, 0x0(%[skin])\n"
                         "lq $3, 0x10(%[skin])\n"
                         "1:\n"
                         "qmtc2.i $2, $vf11\n"
                         "qmtc2 $3, $vf15\n"
                         "vcallms 0xdd0\n"
                         "qmfc2 $2, $vf16\n"
                         "qmfc2 $3, $vf12\n"
                         "sq $2, 0x10(%[out])\n"
                         "sq $3, 0x0(%[out])\n"
                         "addi %[out], %[out], 0x40\n"
                         "addi %[skin], %[skin], 0x20\n"
                         "lq $2, 0x0(%[skin])\n"
                         "lq $3, 0x10(%[skin])\n"
                         "bne %[skin], %[end], 1b\n"
                         "nop\n"
                         "nop\n"
                         "qmfc2.i $2, $vf16\n"
                         "qmfc2 $3, $vf12\n"
                         "sq $2, 0x10(%[out])\n"
                         "sq $3, 0x0(%[out])\n"
                         ".set reorder\n"
                         : [skin] "+r"(pSkin), [out] "+r"(pOut)
                         : [first] "r"(&first), [world] "r"(&world), [end] "r"(pEnd)
                         : "$2", "$3", "memory");
    }
    SyncDCache(mVerts.data(), ByteBefore(mVerts.data() + mVerts.size()));
    return g_identityXfm;
}

int PsMesh::DrawShowing() {
    Sphere worldSphere;
    if (!CheckLod(worldSphere)) {
        return 0;
    }
    const Transform &xfm = SkinXfm();
    auto *pGeom = static_cast<PsMesh *>(mGeomOwner);

    if (pGeom->mFacesPacket != nullptr) {
        int nPass = 0;
        do {
            SetZTest(nPass);
            if (mMat != nullptr) {
                nPass = static_cast<PsMat *>(mMat)->Select(nPass);
            } else {
                PsMat::SelectDefault();
            }
            DrawFaces(xfm);
        } while (nPass != 0);
    }

    if (pGeom->mEdgesPacket != nullptr) {
        SetZTest(kPassEdges);
        if (mMat != nullptr) {
            PsMat::SelectAlphaBlend();
        } else if (pGeom->mFaces.empty()) {
            PsMat::SelectDefault();
        }
        DrawEdges(xfm);
    }

    if (mMutable == 0) {
        ThePs.Flush();
    }
    return 1;
}

void PsMesh::FacesPacket(MemHandle *&pPacket, int &nSetupQwords, int &nVuEnd) const {
    const auto *pGeom = static_cast<const PsMesh *>(mGeomOwner);
    pPacket = pGeom->mFacesPacket;
    nSetupQwords = pGeom->mFacesSetupQwords;
    nVuEnd = pGeom->mFacesVuEnd;
}

void PsMesh::Init() {
    TheManager.SetClassFactory(RndMesh::sClassName, PsMesh::New);
    g_identityXfm.mBasisX.x = 1.0f;
    g_identityXfm.mBasisX.y = 0.0f;
    g_identityXfm.mBasisX.z = 0.0f;
    g_identityXfm.mBasisY.x = 0.0f;
    g_identityXfm.mBasisY.y = 1.0f;
    g_identityXfm.mBasisY.z = 0.0f;
    g_identityXfm.mBasisZ.x = 0.0f;
    g_identityXfm.mBasisZ.y = 0.0f;
    g_identityXfm.mBasisZ.z = 1.0f;
    g_identityXfm.mTranslation = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
}

// The unit's static initialiser at NTSC-U/C: 0x00216318, PAL: 0x0021f130, and its global
// constructor at NTSC-U/C: 0x00216320, PAL: 0x0021f138, are empty.
