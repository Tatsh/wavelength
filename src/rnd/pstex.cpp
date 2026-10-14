#include "rnd/pstex.h"

#include <cstdint>

#include "os/debug.h"
#include "rnd/dmapacket.h"
#include "rnd/ps.h"
#include "rnd/rndmanager.h"

namespace {

// GS memory has 16384 blocks of 256 bytes. A page is 32 blocks, the largest alignment.
constexpr int kGsBlockCount = 0x4000;
constexpr int kPageShift = 5;
constexpr int kAlignShift16 = 4;
constexpr int kAlignShift8 = 3;
constexpr int kAlignShift4 = 2;
constexpr int kAlignPageBlocks = 16;
constexpr int kAlign16Blocks = 8;
constexpr int kAlign8Blocks = 4;
constexpr int kAlign4Blocks = 2;
constexpr int kWordsPerBlockShift = 6;
constexpr int kBlockWordsRound = (1 << kWordsPerBlockShift) - 1;

// The GS registers a texture writes.
constexpr int kGsRegTex01 = 0x06;
constexpr int kGsRegTex11 = 0x14;
constexpr int kGsRegXyOffset1 = 0x18;
constexpr int kGsRegMipTbp11 = 0x34;
constexpr int kGsRegMipTbp21 = 0x36;
constexpr int kGsRegTexFlush = 0x3f;
constexpr int kGsRegDthe = 0x45;
constexpr int kGsRegFrame1 = 0x4c;
constexpr unsigned long long kGsRegBitBltBuf = 0x50;
constexpr unsigned long long kGsRegTrxPos = 0x51;
constexpr unsigned long long kGsRegTrxReg = 0x52;
constexpr unsigned long long kGsRegTrxDir = 0x53;

// The order of mRanges and of the bits of mDirty. The CLUT comes first when the bitmap has a
// palette, and the texture of each mipmap follows. Each slot sets one base pointer.
enum Slot {
    kSlotClut = 0,
    kSlotTbp0 = 1,
    kSlotTbp1 = 2,
    kSlotTbp2 = 3,
    kSlotTbp3 = 4,
    kSlotTbp4 = 5,
    kSlotTbp5 = 6,
    kSlotTbp6 = 7,
    kSlotCount = 8,
};

// The mipmap levels whose buffer width SyncRegisters() records.
enum Level {
    kLevel0 = 0,
    kLevel1 = 1,
    kLevel2 = 2,
    kLevel3 = 3,
    kLevel4 = 4,
    kLevel5 = 5,
    kLevel6 = 6,
    kLevelCount = 7,
};

// The bits of mDirty UnlockBitmap() sets, and the lock flag that marks a changed palette.
constexpr unsigned int kDirtyFirst = 1;
constexpr unsigned int kDirtyFirstTwo = 3;
constexpr int kLockPalette = 4;

// A GS base pointer and buffer width are 14 and 6 bits.
constexpr unsigned long long kBaseMask = 0x3fff;
constexpr unsigned long long kWidthMask = 0x3f;

// TEX0 fields.
constexpr int kTex0TbwShift = 14;
constexpr int kTex0PsmShift = 20;
constexpr unsigned long long kTex0PsmMask = 0x3f;
constexpr int kTex0TwShift = 26;
constexpr int kTex0ThShift = 30;
constexpr unsigned long long kTex0LogSizeMask = 0xf;
constexpr unsigned long long kTex0Tcc = 1ULL << 34;
constexpr int kTex0TfxShift = 35;
constexpr unsigned long long kTex0TfxMask = 3;
constexpr int kTex0CbpShift = 37;
constexpr int kTex0CpsmShift = 51;
constexpr unsigned long long kTex0CpsmMask = 0xf;
constexpr unsigned long long kTex0Csm = 1ULL << 55;
constexpr int kTex0CsaShift = 56;
constexpr unsigned long long kTex0CsaMask = 0x1f;
constexpr int kTex0CldShift = 61;
constexpr unsigned long long kTex0CldMask = 7;
constexpr unsigned long long kTex0CldLoad = 1;

// GS pixel storage modes.
constexpr unsigned long long kPsmCt32 = 0x00;
constexpr unsigned long long kPsmCt16 = 0x02;
constexpr unsigned long long kPsmT8 = 0x13;
constexpr unsigned long long kPsmT4 = 0x14;

// TEX1 fields. The magnification is linear, and the minification is linear, or trilinear across
// the mipmaps when there are any.
constexpr unsigned long long kTex1Lcm = 1;
constexpr int kTex1MxlShift = 2;
constexpr unsigned long long kTex1MxlMask = 7;
constexpr unsigned long long kTex1MmagLinear = 1ULL << 5;
constexpr int kTex1MminShift = 6;
constexpr unsigned long long kTex1MminMask = 7;
constexpr unsigned long long kTex1MminLinear = 1;
constexpr unsigned long long kTex1MminLinearMipmapLinear = 4;
constexpr unsigned long long kTex1Mtba = 1ULL << 9;
constexpr int kTex1LShift = 19;
constexpr unsigned long long kTex1LMask = 3;
constexpr int kTex1KShift = 32;
constexpr unsigned long long kTex1KMask = 0xfff;
constexpr unsigned long long kTex1SelectMask = 0xfff001803fdULL;

// MIPTBP fields, a base pointer and a buffer width for each of three mipmaps.
constexpr int kMipTbpBase1Shift = 0;
constexpr int kMipTbpBase2Shift = 20;
constexpr int kMipTbpBase3Shift = 40;
constexpr int kMipTbpWidth1Shift = 14;
constexpr int kMipTbpWidth2Shift = 34;
constexpr int kMipTbpWidth3Shift = 54;

// FRAME_1 and XYOFFSET_1 of a render target, and the DTHE bit.
constexpr int kFramePageShift = 5;
constexpr int kFrameFbwShift = 16;
constexpr int kFramePsmShift = 24;
constexpr unsigned long long kFrameSelectMask = 0x3f3f01ff;
constexpr int kGsCoordinateCentre = 0x800;
constexpr int kGsSubpixelShift = 4;
constexpr unsigned long long kXyOffsetMask = 0xffff;
constexpr int kXyOffsetYShift = 32;
constexpr unsigned long long kXyOffsetSelectMask = 0x0000ffff0000ffffULL;
constexpr unsigned long long kDthe = 1;

// BITBLTBUF, TRXREG, and TRXPOS fields.
constexpr int kBitBltDbpShift = 32;
constexpr int kBitBltDbwShift = 48;
constexpr int kBitBltDpsmShift = 56;
constexpr int kTrxRegHeightShift = 32;
constexpr int kTrxPosDsaxShift = 32;
constexpr int kTrxPosDsayShift = 48;

// A buffer width counts 64 pixels. A texture of 8 bits per pixel or fewer counts in pairs of 64.
constexpr int kMaxPaletteBpp = 8;
constexpr int kWidthRound = 63;
constexpr int kWidthShift = 6;
constexpr int kPaletteWidthRound = 127;
constexpr int kPaletteWidthShift = 7;

// A movie uploads in blocks of 16 by 16 pixels of 32 or 16 bits, 64 or 32 quadwords of pixels,
// with seven quadwords of tags for each block.
constexpr int kMovieBlockSize = 16;
constexpr int kMovieBlockPixelsShift = 8;
constexpr int kMovieBlockQwords32 = 0x40;
constexpr int kMovieBlockQwords16 = 0x20;
constexpr int kMovieBlockTagQwords = 7;
constexpr int kQwordBytes = 16;
constexpr int kMoviePacketAlign = 0x40;
constexpr int kBitsPerQword = 128;
constexpr int kBitsPerQwordShift = 7;

// The CLUT is uploaded as a 32-bit image, 8 by 2 for 16 entries and 16 by 16 for 256.
constexpr int kClutBpp = 32;
constexpr int kSmallClutBpp = 4;
constexpr int kSmallClutWidth = 8;
constexpr int kSmallClutHeight = 2;
constexpr int kClutWidth = 16;
constexpr int kClutHeight = 16;

// The blocks of a CLUT of 256 entries and of one of 16.
constexpr unsigned short kClutBlocks = 4;
constexpr unsigned short kSmallClutBlocks = 1;

// Bitmap depths.
constexpr int kBpp4 = 4;
constexpr int kBpp8 = 8;
constexpr int kBpp16 = 16;
constexpr int kBpp32 = 32;

// The side of a block in pixels, as a power of two, and the blocks across and down a page.
constexpr int kBlockShift8 = 3;
constexpr int kBlockShift16 = 4;
constexpr int kBlockShift32 = 5;
constexpr int kPageBlocksWide = 8;
constexpr int kPageBlocksHigh = 4;

// A rendered texture of fewer than 16 bits per pixel becomes 16 bits.
constexpr int kMinRenderedBpp = kBpp16;
constexpr int kRenderedOrder = RndBitmap::kOrderRGBA | RndBitmap::kOrderGs;

// GIF and DMA tags.
constexpr int kGifFlgPacked = 0;
constexpr int kGifFlgImage = 2;
constexpr int kGifAdRegs = 1;
constexpr unsigned long long kGifRegAd = 0xe;
constexpr int kDmaTagIdCnt = 1;
constexpr int kDmaTagIdRef = 3;
constexpr int kDmaChannelToSpr = 9;

constexpr int kLowDword = 0;
constexpr int kHighDword = 1;

inline void PushQuadword(unsigned long long qwLow, unsigned long long qwHigh) {
    DmaPacket &packet = DmaPacket::shared();
    packet.mCursor->mDword[kLowDword] = qwLow;
    packet.mCursor->mDword[kHighDword] = qwHigh;
    ++packet.mCursor;
}

inline unsigned int DmaAddressOf(const void *pAddress) {
    return static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pAddress));
}

inline int RoundUpToAlignment(int nStart, int nShift) {
    return static_cast<int>(((static_cast<unsigned int>(nStart + (1 << nShift) - 1)) >> nShift)
                            << nShift);
}

inline unsigned long long SetField(unsigned long long qwValue,
                                   unsigned long long qwField,
                                   int nShift,
                                   unsigned long long qwMask) {
    return (qwValue & ~(qwMask << nShift)) | ((qwField & qwMask) << nShift);
}

inline int BlocksAcross(int nPixels, int nShift) {
    return (nPixels + (1 << nShift) - 1) >> nShift;
}

inline int Log2Floor(int nSize) {
    int nLog = 0;
    while (nSize >= 2) {
        nSize >>= 1;
        ++nLog;
    }
    return nLog;
}

// The buffer width of an image, in units of 64 pixels.
inline int BufferWidth(int nBpp, int nWidth) {
    if (nBpp < kMaxPaletteBpp + 1) {
        return ((nWidth + kPaletteWidthRound) >> kPaletteWidthShift) * 2;
    }
    return (nWidth + kWidthRound) >> kWidthShift;
}

} // namespace

std::list<PsTex::FreeRange> PsTex::sFreeRanges;
std::list<PsTex::UsedRange *> PsTex::sUsedRanges;
int PsTex::sStamp;
int PsTex::sEvictStamp;

// The constructor and the destructor of sFreeRanges and sUsedRanges are the unit's static
// initialiser, its global constructor, and its global destructor.
// NTSC-U/C: 0x00219128, PAL: 0x00221f40
// NTSC-U/C: 0x00219228, PAL: 0x00222040
// NTSC-U/C: 0x00219248, PAL: 0x00222060

int PsTex::UsedRange::AlignShift() const {
    if (mPinned) {
        return kPageShift;
    }
    if (mCount > kAlignPageBlocks) {
        return kPageShift;
    }
    if (mCount > kAlign16Blocks) {
        return kAlignShift16;
    }
    if (mCount > kAlign8Blocks) {
        return kAlignShift8;
    }
    if (mCount > kAlign4Blocks) {
        return kAlignShift4;
    }
    return 1 < mCount;
}

bool PsTex::UsedRange::AllocFree() {
    const int nShift = AlignShift();
    int nBestPad = 0;
    int nBestWaste = kGsBlockCount;
    auto best = sFreeRanges.end();
    for (auto it = sFreeRanges.begin(); it != sFreeRanges.end(); ++it) {
        const int nPad = RoundUpToAlignment(it->mStart, nShift) - it->mStart;
        if (it->mCount < mCount + nPad) {
            continue;
        }
        // The waste ignores the padding, and an exact fit is taken without it.
        const int nWaste = it->mCount - mCount;
        if (nWaste == 0) {
            mStart = it->mStart;
            sFreeRanges.erase(it);
            return true;
        }
        if (nWaste < nBestWaste) {
            best = it;
            nBestWaste = nWaste;
            nBestPad = nPad;
        }
    }
    if (best == sFreeRanges.end()) {
        return false;
    }

    if (nBestPad != 0) {
        sFreeRanges.insert(best, FreeRange{best->mStart, static_cast<unsigned short>(nBestPad)});
        best->mCount -= nBestPad;
        best->mStart += nBestPad;
    }
    mStart = best->mStart;
    best->mCount -= mCount;
    best->mStart += mCount;
    if (best->mCount == 0) {
        sFreeRanges.erase(best);
    }
    return true;
}

bool PsTex::UsedRange::AllocUsed() {
    const int nShift = AlignShift();
    int nBestPad = 0;
    int nBestWaste = kGsBlockCount;
    int nBestFrame = 0;
    auto best = sUsedRanges.end();
    for (auto it = sUsedRanges.begin(); it != sUsedRanges.end(); ++it) {
        UsedRange *pRange = *it;
        const int nPad = RoundUpToAlignment(pRange->mStart, nShift) - pRange->mStart;
        if (pRange->mCount < mCount + nPad) {
            continue;
        }
        if (pRange->mPinned) {
            continue;
        }
        const int nFrame = pRange->mFrame;
        if (nFrame == sStamp) {
            continue;
        }
        const int nWaste = pRange->mCount - mCount;
        if (nWaste < nBestWaste) {
            best = it;
            nBestWaste = nWaste;
            nBestPad = nPad;
            nBestFrame = nFrame;
        } else if (nWaste == nBestWaste && nBestFrame < nFrame) {
            best = it;
            nBestFrame = nFrame;
            nBestPad = nPad;
        }
    }
    if (best == sUsedRanges.end()) {
        return false;
    }

    UsedRange *pVictim = *best;
    if (nBestPad != 0) {
        FreeBlocks(pVictim->mStart, nBestPad);
        pVictim->mCount -= nBestPad;
        pVictim->mStart += nBestPad;
    }
    mStart = pVictim->mStart;
    pVictim->mStart = 0;
    mFrame = sStamp;
    FreeBlocks(mStart + mCount, pVictim->mCount - mCount);
    sUsedRanges.erase(best);
    return true;
}

void PsTex::UsedRange::Release() {
    if (mStart == 0) {
        return;
    }
    FreeBlocks(mStart, mCount);
    mStart = 0;
    sUsedRanges.remove(this);
}

PsTex::PsTex(const char *pszName)
    : RndTex(pszName), mRanges(), mLockFlags(0), mDirty(0), mMoviePacket(nullptr) {
}

PsTex::~PsTex() {
    ReleaseSurfaces();
    if (mMoviePacket != nullptr) {
        PoolMemFree(mMoviePacket);
        mMoviePacket = nullptr;
    }
}

RndBitmap *PsTex::LockBitmap(int nFlags) {
    mLockFlags = nFlags;
    return &mBitmap;
}

void PsTex::UnlockBitmap() {
    const unsigned int nDirty = mDirty;
    mDirty = nDirty | kDirtyFirst;
    if ((mLockFlags & kLockPalette) != 0) {
        mDirty = nDirty | kDirtyFirstTwo;
    }
    mLockFlags = 0;
}

void PsTex::BeginFrame() {
    for (auto it = sUsedRanges.begin(); it != sUsedRanges.end();) {
        UsedRange *pRange = *it;
        ++it;
        if (pRange->mFrame < sEvictStamp) {
            pRange->Release();
        }
    }
    sEvictStamp = sStamp;
}

int PsTex::FreeBlockCount() {
    int nCount = 0;
    for (const auto &range : sFreeRanges) {
        nCount += range.mCount;
    }
    return nCount;
}

void PsTex::ReserveDisplayMemory() {
    const int nReserved = (ThePs.GetReservedVramWords() + kBlockWordsRound) >> kWordsPerBlockShift;
    const FreeRange free{static_cast<unsigned short>(nReserved),
                         static_cast<unsigned short>(kGsBlockCount - nReserved)};
    sFreeRanges.clear();
    sFreeRanges.push_back(free);
    for (UsedRange *pRange : sUsedRanges) {
        pRange->mStart = 0;
    }
    sUsedRanges.clear();
}

void PsTex::Place() {
    int nSlot = mBitmap.mPalette == nullptr ? kSlotTbp0 : kSlotClut;
    for (auto it = mRanges.begin(); it != mRanges.end(); ++it, ++nSlot) {
        UsedRange &range = *it;
        if (range.mStart != 0) {
            continue;
        }
        if (!range.AllocFree() && !range.AllocUsed()) {
            DebugPrint("Can't swap %s, clearing Vram\n", mName.c_str());
            ReserveDisplayMemory();
            range.AllocFree();
        }
        sUsedRanges.push_back(&range);
        mDirty |= 1U << nSlot;

        const unsigned long long qwBase = range.mStart & kBaseMask;
        switch (nSlot) {
        case kSlotClut:
            mTex0 = SetField(mTex0, qwBase, kTex0CbpShift, kBaseMask);
            break;
        case kSlotTbp0:
            mTex0 = SetField(mTex0, qwBase, 0, kBaseMask);
            break;
        case kSlotTbp1:
            mMipTbp1 = SetField(mMipTbp1, qwBase, kMipTbpBase1Shift, kBaseMask);
            break;
        case kSlotTbp2:
            mMipTbp1 = SetField(mMipTbp1, qwBase, kMipTbpBase2Shift, kBaseMask);
            break;
        case kSlotTbp3:
            mMipTbp1 = SetField(mMipTbp1, qwBase, kMipTbpBase3Shift, kBaseMask);
            break;
        case kSlotTbp4:
            mMipTbp2 = SetField(mMipTbp2, qwBase, kMipTbpBase1Shift, kBaseMask);
            break;
        case kSlotTbp5:
            mMipTbp2 = SetField(mMipTbp2, qwBase, kMipTbpBase2Shift, kBaseMask);
            break;
        case kSlotTbp6:
            mMipTbp2 = SetField(mMipTbp2, qwBase, kMipTbpBase3Shift, kBaseMask);
            break;
        default:
            break;
        }
    }
}

void PsTex::BuildMoviePacket(bool bBuild) {
    if (mMoviePacket != nullptr) {
        PoolMemFree(mMoviePacket);
        mMoviePacket = nullptr;
    }
    if (!bBuild || mBitmap.mPixels == nullptr) {
        return;
    }

    const int nBlocks = (mWidth * mHeight) >> kMovieBlockPixelsShift;
    mMoviePacket = static_cast<DmaQuadword *>(PoolMemAlloc(
        nBlocks * kMovieBlockTagQwords * kQwordBytes, "mMoviePacket", kMoviePacketAlign));
    DmaPacket packet;
    packet.mBase = nullptr; // Yes, the binary clears the base that Init() then replaces.
    packet.Init(mMoviePacket, kDmaChannelToSpr, 1);

    const int nBlockQwords = ((mTex0 >> kTex0PsmShift) & kTex0PsmMask) == kPsmCt32 ?
                                 kMovieBlockQwords32 :
                                 kMovieBlockQwords16;
    const unsigned char *pPixels = mBitmap.mPixels;
    for (int nY = 0; nY < mHeight; nY += kMovieBlockSize) {
        const unsigned long long qwY = static_cast<unsigned long long>(nY) << kTrxPosDsayShift;
        for (int nX = 0; nX < mWidth; nX += kMovieBlockSize) {
            packet.OpenDmaTag();
            packet.OpenGifTag(kGifFlgPacked, kGifAdRegs, kGifRegAd, 0);
            packet.mCursor->mDword[kLowDword] =
                (static_cast<unsigned long long>(nX) << kTrxPosDsaxShift) | qwY;
            packet.mCursor->mDword[kHighDword] = kGsRegTrxPos;
            ++packet.mCursor;
            packet.mCursor->mDword[kLowDword] = 0;
            packet.mCursor->mDword[kHighDword] = kGsRegTrxDir;
            ++packet.mCursor;
            packet.CloseGifTag(0, 0);
            packet.OpenGifTag(kGifFlgImage, 0, 0, 0);
            packet.CloseGifTag(1, nBlockQwords);
            packet.CloseDmaTag(kDmaTagIdCnt, 0, 0);
            packet.OpenDmaTag();
            packet.CloseDmaTag(kDmaTagIdRef, DmaAddressOf(pPixels), nBlockQwords);
            pPixels += nBlockQwords * kQwordBytes;
        }
    }
}

void PsTex::UploadMovie() {
    DmaPacket &frame = DmaPacket::shared();
    frame.OpenGifTag(kGifFlgPacked, kGifAdRegs, kGifRegAd, 0);
    PushQuadword(((mTex0 & kBaseMask) << kBitBltDbpShift) |
                     (((mTex0 >> kTex0TbwShift) & kWidthMask) << kBitBltDbwShift) |
                     (((mTex0 >> kTex0PsmShift) & kTex0PsmMask) << kBitBltDpsmShift),
                 kGsRegBitBltBuf);
    PushQuadword(kMovieBlockSize |
                     (static_cast<unsigned long long>(kMovieBlockSize) << kTrxRegHeightShift),
                 kGsRegTrxReg);
    frame.CloseGifTag(0, 0);

    int nRemaining = (mWidth * mHeight) >> kMovieBlockPixelsShift;
    DmaQuadword *pSource = mMoviePacket;
    DmaPacket packet;
    packet.mBase = nullptr; // Yes, the binary clears the base that Init() then replaces.
    while (nRemaining != 0) {
        const int nRoom = ThePs.PacketQwordsFree() / kMovieBlockTagQwords;
        const int nBlocks = nRoom < nRemaining ? nRoom : nRemaining;
        nRemaining -= nBlocks;
        const int nQwords = nBlocks * kMovieBlockTagQwords;
        frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
        DmaQuadword *pDest = frame.mCursor;
        packet.Init(pSource, kDmaChannelToSpr, 1);
        pSource += nQwords;
        packet.mCursor = pSource;
        packet.Send(DmaPacket::kModeNormal, DmaAddressOf(pDest));
        frame.mCursor = pDest + nQwords;
        ThePs.Flush();
    }
}

void PsTex::UploadImage(
    int nDbp, int nPsm, int nBpp, int nWidth, int nHeight, const unsigned char *pPixels) {
    if (pPixels == nullptr) {
        return;
    }
    DmaPacket &frame = DmaPacket::shared();
    if (frame.mGifTag == nullptr || frame.mGifTag->mDword[kHighDword] != kGifRegAd) {
        frame.OpenGifTag(kGifFlgPacked, kGifAdRegs, kGifRegAd, 0);
    }
    const auto qwWidth = static_cast<unsigned long long>(BufferWidth(nBpp, nWidth));
    PushQuadword((static_cast<unsigned long long>(nDbp) << kBitBltDbpShift) |
                     (qwWidth << kBitBltDbwShift) |
                     (static_cast<unsigned long long>(nPsm) << kBitBltDpsmShift),
                 kGsRegBitBltBuf);
    PushQuadword(static_cast<unsigned long long>(nWidth) |
                     (static_cast<unsigned long long>(nHeight) << kTrxRegHeightShift),
                 kGsRegTrxReg);
    PushQuadword(0, kGsRegTrxPos);
    PushQuadword(0, kGsRegTrxDir);

    const int nQwords = (nWidth * nHeight * nBpp) >> kBitsPerQwordShift;
    frame.OpenGifTag(kGifFlgImage, 0, 0, 0);
    frame.CloseGifTag(1, nQwords);
    frame.CloseDmaTag(kDmaTagIdCnt, 0, 0);
    frame.OpenDmaTag();
    frame.CloseDmaTag(kDmaTagIdRef, DmaAddressOf(pPixels), nQwords);
    frame.OpenDmaTag();
}

void PsTex::Upload() {
    Place();
    const RndBitmap *pMip = &mBitmap;
    int nSlot = mBitmap.mPalette == nullptr ? kSlotTbp0 : kSlotClut;
    for (auto it = mRanges.begin(); it != mRanges.end(); ++it, ++nSlot) {
        if (((mDirty >> nSlot) & 1) == 0) {
            continue;
        }
        if (nSlot == kSlotClut) {
            const bool bSmall = mBpp == kSmallClutBpp;
            UploadImage(static_cast<int>((mTex0 >> kTex0CbpShift) & kBaseMask),
                        static_cast<int>((mTex0 >> kTex0CpsmShift) & kTex0CpsmMask),
                        kClutBpp,
                        bSmall ? kSmallClutWidth : kClutWidth,
                        bSmall ? kSmallClutHeight : kClutHeight,
                        mBitmap.mPalette);
        } else if (mMoviePacket != nullptr) {
            UploadMovie();
        } else {
            UploadImage(it->mStart,
                        static_cast<int>((mTex0 >> kTex0PsmShift) & kTex0PsmMask),
                        mBpp,
                        pMip->mWidth,
                        pMip->mHeight,
                        pMip->mPixels);
            pMip = pMip->mMip;
        }
    }
    mDirty = 0;
    DmaPacket::shared().OpenGifTag(kGifFlgPacked, kGifAdRegs, kGifRegAd, 0);
    PushQuadword(0, kGsRegTexFlush);
}

void PsTex::FreeBlocks(int nStart, int nCount) {
    if (nCount == 0) {
        return;
    }
    auto prev = sFreeRanges.end();
    auto next = sFreeRanges.begin();
    while (next != sFreeRanges.end() && !(nStart < next->mStart)) {
        prev = next;
        ++next;
    }

    if (prev != sFreeRanges.end() && prev->mStart + prev->mCount == nStart) {
        prev->mCount += nCount;
        if (next != sFreeRanges.end() && nStart + nCount == next->mStart) {
            prev->mCount += next->mCount;
            sFreeRanges.erase(next);
        }
    } else if (next != sFreeRanges.end() && nStart + nCount == next->mStart) {
        next->mStart = nStart;
        next->mCount += nCount;
    } else {
        sFreeRanges.insert(
            next,
            FreeRange{static_cast<unsigned short>(nStart), static_cast<unsigned short>(nCount)});
    }
}

void PsTex::BindAsRenderTarget() {
    Place();
    const unsigned long long qwFrame =
        ((mTex0 & kBaseMask) >> kFramePageShift) |
        (((mTex0 >> kTex0TbwShift) & kWidthMask) << kFrameFbwShift) |
        (((mTex0 >> kTex0PsmShift) & kTex0PsmMask) << kFramePsmShift);
    ThePs.SetGsReg(kGsRegFrame1, qwFrame, kFrameSelectMask);

    const unsigned long long qwOffsetX =
        static_cast<unsigned long long>((kGsCoordinateCentre - (mWidth >> 1)) << kGsSubpixelShift) &
        kXyOffsetMask;
    const unsigned long long qwOffsetY =
        static_cast<unsigned long long>((kGsCoordinateCentre - (mHeight >> 1))
                                        << kGsSubpixelShift) &
        kXyOffsetMask;
    ThePs.SetGsReg(
        kGsRegXyOffset1, qwOffsetX | (qwOffsetY << kXyOffsetYShift), kXyOffsetSelectMask);

    const bool bSixteenBit = ((mTex0 >> kTex0PsmShift) & kTex0PsmMask) == kPsmCt16;
    ThePs.SetGsReg(kGsRegDthe, bSixteenBit, kDthe);
}

void PsTex::Init() {
    TheManager.mClasses[RndTex::sClassName] = PsTex::New;
    sEvictStamp = 0;
    sStamp = 0;
}

int PsTex::NumBlocks(const RndBitmap &bitmap) {
    const int nWidth = bitmap.mWidth;
    const int nHeight = bitmap.mHeight;
    // The block grid of the bitmap, one side in each variable, in the order the GS page shapes of
    // each depth make convenient.
    int nPrimary;
    int nSecondary;
    switch (bitmap.mBpp) {
    case kBpp8:
        nPrimary = BlocksAcross(nWidth, kBlockShift16);
        nSecondary = BlocksAcross(nHeight, kBlockShift16);
        break;
    case kBpp4:
        nPrimary = BlocksAcross(nHeight, kBlockShift16);
        nSecondary = BlocksAcross(nWidth, kBlockShift32);
        break;
    case kBpp16:
        nPrimary = BlocksAcross(nHeight, kBlockShift8);
        nSecondary = BlocksAcross(nWidth, kBlockShift16);
        break;
    default:
        nPrimary = BlocksAcross(nWidth, kBlockShift8);
        nSecondary = BlocksAcross(nHeight, kBlockShift8);
        break;
    }

    if (nPrimary > kPageBlocksWide) {
        return nPrimary * (nSecondary < kPageBlocksHigh ? kPageBlocksHigh : nSecondary);
    }
    if (nSecondary > kPageBlocksHigh) {
        return (nPrimary < kPageBlocksWide ? kPageBlocksWide : nPrimary) * nSecondary;
    }
    const int nHalf = nPrimary >> 1;
    return (nPrimary < nSecondary ? nSecondary : nPrimary) *
           (nSecondary < nHalf ? nHalf : nSecondary);
}

void PsTex::SyncRegisters() {
    if (mBitmap.mPixels == nullptr) {
        return;
    }

    int nBpp = mBitmap.mBpp;
    if (mRendered != 0 && nBpp < kMinRenderedBpp) {
        // The test always selects 16 for a depth below 16.
        nBpp = nBpp > kBpp32 - 1 ? kBpp32 : kMinRenderedBpp;
        mBitmap.Create(mBitmap.mWidth, mBitmap.mHeight, 0, nBpp, kRenderedOrder);
    }

    unsigned long long qwPsm;
    if (nBpp == kBpp4) {
        qwPsm = kPsmT4;
    } else if (nBpp == kBpp8) {
        qwPsm = kPsmT8;
    } else if (nBpp == kBpp16) {
        qwPsm = kPsmCt16;
    } else {
        qwPsm = kPsmCt32;
    }
    mTex0 = SetField(mTex0, qwPsm, kTex0PsmShift, kTex0PsmMask);
    mTex0 = SetField(mTex0, Log2Floor(mWidth), kTex0TwShift, kTex0LogSizeMask);

    unsigned long long qwTex0 = SetField(mTex0, Log2Floor(mHeight), kTex0ThShift, kTex0LogSizeMask);
    qwTex0 |= kTex0Tcc;
    qwTex0 = SetField(qwTex0, 0, kTex0CbpShift, kBaseMask);
    qwTex0 = SetField(qwTex0, 0, kTex0CpsmShift, kTex0CpsmMask);
    qwTex0 &= ~kTex0Csm;
    qwTex0 = SetField(qwTex0, 0, kTex0CsaShift, kTex0CsaMask);
    qwTex0 = SetField(qwTex0, kTex0CldLoad, kTex0CldShift, kTex0CldMask);
    mTex1 &= ~kTex1Lcm;
    mTex0 = qwTex0;

    unsigned long long qwTex1 =
        SetField(mTex1, mBitmap.NumMips(), kTex1MxlShift, kTex1MxlMask) | kTex1MmagLinear;
    const unsigned long long qwMin = ((qwTex1 >> kTex1MxlShift) & kTex1MxlMask) != 0 ?
                                         kTex1MminLinearMipmapLinear :
                                         kTex1MminLinear;
    qwTex1 = SetField(qwTex1, qwMin, kTex1MminShift, kTex1MminMask);
    qwTex1 &= ~kTex1Mtba;
    qwTex1 = SetField(qwTex1, 0, kTex1LShift, kTex1LMask);
    mTex1 = SetField(qwTex1, mMipMapK, kTex1KShift, kTex1KMask);

    UsedRange range;
    if (mBitmap.mPalette != nullptr) {
        range.mStart = 0;
        range.mPinned = mRendered & 1;
        range.mFrame = 0;
        range.mCount = mBpp == kBpp8 ? kClutBlocks : kSmallClutBlocks;
        mRanges.push_back(range);
    }

    int nLevel = kLevel0;
    for (const RndBitmap *pMip = &mBitmap; pMip != nullptr; pMip = pMip->mMip, ++nLevel) {
        range.mCount = static_cast<unsigned short>(NumBlocks(*pMip));
        range.mStart = 0;
        range.mPinned = mRendered & 1;
        range.mFrame = 0;
        mRanges.push_back(range);

        const unsigned long long qwWidth = BufferWidth(nBpp, pMip->mWidth);
        switch (nLevel) {
        case kLevel0:
            mTex0 = SetField(mTex0, qwWidth, kTex0TbwShift, kWidthMask);
            break;
        case kLevel1:
            mMipTbp1 = SetField(mMipTbp1, qwWidth, kMipTbpWidth1Shift, kWidthMask);
            break;
        case kLevel2:
            mMipTbp1 = SetField(mMipTbp1, qwWidth, kMipTbpWidth2Shift, kWidthMask);
            break;
        case kLevel3:
            mMipTbp1 = SetField(mMipTbp1, qwWidth, kMipTbpWidth3Shift, kWidthMask);
            break;
        case kLevel4:
            mMipTbp2 = SetField(mMipTbp2, qwWidth, kMipTbpWidth1Shift, kWidthMask);
            break;
        case kLevel5:
            mMipTbp2 = SetField(mMipTbp2, qwWidth, kMipTbpWidth2Shift, kWidthMask);
            break;
        case kLevel6:
            mMipTbp2 = SetField(mMipTbp2, qwWidth, kMipTbpWidth3Shift, kWidthMask);
            break;
        default:
            break;
        }
    }
    mLockFlags = 0;
}

void PsTex::ReleaseSurfaces() {
    mBitmap.Reset();
    for (auto &range : mRanges) {
        range.Release();
    }
    mRanges.clear();
}

void PsTex::Select(int nTexFunction) {
    if (mBitmap.mPixels == nullptr) {
        return;
    }
    Upload();
    mTex0 =
        SetField(mTex0, static_cast<unsigned long long>(nTexFunction), kTex0TfxShift, kTex0TfxMask);
    ThePs.SetGsReg(kGsRegTex01, mTex0, 0);
    ThePs.SetGsReg(kGsRegTex11, mTex1, kTex1SelectMask);
    const int nMips = mBitmap.NumMips();
    if (nMips != 0) {
        ThePs.SetGsReg(kGsRegMipTbp11, mMipTbp1, 0);
    }
    if (nMips >= kLevel4) {
        ThePs.SetGsReg(kGsRegMipTbp21, mMipTbp2, 0);
    }
    for (auto &range : mRanges) {
        range.mFrame = sStamp;
    }
    ++sStamp;
}
