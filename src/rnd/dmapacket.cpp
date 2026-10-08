#include "rnd/dmapacket.h"

#include <cstdint>

namespace {

constexpr int kDmaChannelCount = 10;

// The scratchpad channels. From SPR copies the packet out to main memory, and to SPR copies it in.
constexpr int kChannelFromSpr = 8;
constexpr int kChannelToSpr = 9;

// The registers of each channel, indexed by channel.
// NTSC-U/C: 0x003b06a0
volatile unsigned int *const kChcr[kDmaChannelCount] = {
    reinterpret_cast<volatile unsigned int *>(0x10008000),
    reinterpret_cast<volatile unsigned int *>(0x10009000),
    reinterpret_cast<volatile unsigned int *>(0x1000a000),
    reinterpret_cast<volatile unsigned int *>(0x1000b000),
    reinterpret_cast<volatile unsigned int *>(0x1000b400),
    reinterpret_cast<volatile unsigned int *>(0x1000c000),
    reinterpret_cast<volatile unsigned int *>(0x1000c400),
    reinterpret_cast<volatile unsigned int *>(0x1000c800),
    reinterpret_cast<volatile unsigned int *>(0x1000d000),
    reinterpret_cast<volatile unsigned int *>(0x1000d400),
};

// NTSC-U/C: 0x003b06c8
volatile unsigned int *const kMadr[kDmaChannelCount] = {
    reinterpret_cast<volatile unsigned int *>(0x10008010),
    reinterpret_cast<volatile unsigned int *>(0x10009010),
    reinterpret_cast<volatile unsigned int *>(0x1000a010),
    reinterpret_cast<volatile unsigned int *>(0x1000b010),
    reinterpret_cast<volatile unsigned int *>(0x1000b410),
    reinterpret_cast<volatile unsigned int *>(0x1000c010),
    reinterpret_cast<volatile unsigned int *>(0x1000c410),
    reinterpret_cast<volatile unsigned int *>(0x1000c810),
    reinterpret_cast<volatile unsigned int *>(0x1000d010),
    reinterpret_cast<volatile unsigned int *>(0x1000d410),
};

// NTSC-U/C: 0x003b06f0
volatile unsigned int *const kQwc[kDmaChannelCount] = {
    reinterpret_cast<volatile unsigned int *>(0x10008020),
    reinterpret_cast<volatile unsigned int *>(0x10009020),
    reinterpret_cast<volatile unsigned int *>(0x1000a020),
    reinterpret_cast<volatile unsigned int *>(0x1000b020),
    reinterpret_cast<volatile unsigned int *>(0x1000b420),
    reinterpret_cast<volatile unsigned int *>(0x1000c020),
    reinterpret_cast<volatile unsigned int *>(0x1000c420),
    reinterpret_cast<volatile unsigned int *>(0x1000c820),
    reinterpret_cast<volatile unsigned int *>(0x1000d020),
    reinterpret_cast<volatile unsigned int *>(0x1000d420),
};

// Channels without a TADR have a null entry.
// NTSC-U/C: 0x003b0718
volatile unsigned int *const kTadr[kDmaChannelCount] = {
    reinterpret_cast<volatile unsigned int *>(0x10008030),
    reinterpret_cast<volatile unsigned int *>(0x10009030),
    reinterpret_cast<volatile unsigned int *>(0x1000a030),
    nullptr,
    reinterpret_cast<volatile unsigned int *>(0x1000b430),
    nullptr,
    reinterpret_cast<volatile unsigned int *>(0x1000c430),
    nullptr,
    nullptr,
    reinterpret_cast<volatile unsigned int *>(0x1000d430),
};

// Only the scratchpad channels have an SADR.
// NTSC-U/C: 0x003b0740
volatile unsigned int *const kSadr[kDmaChannelCount] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    reinterpret_cast<volatile unsigned int *>(0x1000d080),
    reinterpret_cast<volatile unsigned int *>(0x1000d480),
};

constexpr std::uintptr_t kScratchpadPacket = 0x70000000;

// CHCR.STR, set while the channel transfers.
constexpr unsigned int kChcrStart = 0x100;
// The CHCR values Close() returns, all with STR set: normal mode, chain mode, and chain mode
// that also transfers the tags.
constexpr unsigned int kChcrStartNormal = 0x101;
constexpr unsigned int kChcrStartChain = 0x105;
constexpr unsigned int kChcrStartChainTagged = 0x145;

// DMA tag fields.
constexpr int kDmaTagIdCnt = 1;
constexpr int kDmaTagIdEnd = 7;
constexpr int kDmaTagIdShift = 28;
constexpr int kDmaTagAddressShift = 32;

// An address becomes a DMA address by keeping the low 28 bits and moving the scratchpad selector,
// bit 30, up to bit 31.
constexpr unsigned int kDmaAddressMask = 0x0fffffff;
constexpr unsigned int kScratchpadAddressBit = 0x40000000;

// GIF tag fields, in the low doubleword.
constexpr unsigned long long kGifTagNloopMask = 0x7fff;
constexpr unsigned long long kGifTagEop = 0x8000;
constexpr unsigned long long kGifTagPre = 1ULL << 46;
constexpr int kGifTagPrimShift = 47;
constexpr int kGifTagFlgShift = 58;
constexpr unsigned long long kGifTagFlgMask = 3;
constexpr int kGifTagNregShift = 60;

// The VIF codes ahead of the GIF data: FLUSHE in the first word and DIRECT in the last, whose
// immediate half counts the quadwords that follow.
constexpr unsigned int kVifCodeFlushE = 0x10000000;
constexpr unsigned int kVifCodeDirect = 0x50000000;
constexpr int kVifFirstWord = 0;
constexpr int kVifLastWord = 3;
constexpr int kVifDirectImmediateHalf = 6;

constexpr int kLowDword = 0;
constexpr int kHighDword = 1;

unsigned int DmaAddress(const void *pAddress) {
    const auto nAddress = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pAddress));
    return (nAddress & kDmaAddressMask) | ((nAddress & kScratchpadAddressBit) << 1);
}

} // namespace

DmaPacket &DmaPacket::shared() {
    return *reinterpret_cast<DmaPacket *>(kScratchpadPacket);
}

void DmaPacket::OpenDmaTag() {
    CloseDmaTag(kDmaTagIdCnt, 0, 0);
    mDmaTag = mCursor;
    ++mCursor;
}

void DmaPacket::CloseDmaTag(int nId, unsigned int nAddress, int nQwc) {
    if (mDmaTag == nullptr) {
        return;
    }
    CloseGifTag(1, 0);
    if (nQwc == 0) {
        nQwc = static_cast<int>(mCursor - mDmaTag) - 1;
    }
    const unsigned int nTagAddress =
        (nAddress & kDmaAddressMask) | ((nAddress & kScratchpadAddressBit) << 1);
    mDmaTag->mDword[kLowDword] =
        static_cast<unsigned long long>(nQwc) |
        (static_cast<unsigned long long>(nId) << kDmaTagIdShift) |
        (static_cast<unsigned long long>(nTagAddress) << kDmaTagAddressShift);
    mDmaTag = nullptr;
}

void DmaPacket::CloseGifTag(int bEop, int nLoops) {
    DmaQuadword *pTag = mGifTag;
    if (pTag == nullptr) {
        return;
    }
    if (nLoops != 0) {
        pTag->mDword[kLowDword] = (pTag->mDword[kLowDword] & ~kGifTagNloopMask) |
                                  (static_cast<unsigned long long>(nLoops) & kGifTagNloopMask);
    } else {
        const unsigned long long nTag = pTag->mDword[kLowDword];
        const auto nFlg = static_cast<int>((nTag >> kGifTagFlgShift) & kGifTagFlgMask);
        const unsigned long long nQuadwordsPerLoop = (nTag >> kGifTagNregShift) >> nFlg;
        const auto nData = static_cast<unsigned long long>(mCursor - pTag - 1);
        pTag->mDword[kLowDword] =
            (nTag & ~kGifTagNloopMask) | ((nData / nQuadwordsPerLoop) & kGifTagNloopMask);
    }
    if (mVifDirect != nullptr) {
        if (nLoops != 0) {
            mVifDirect->mHalf[kVifDirectImmediateHalf] += nLoops + 1;
        } else {
            mVifDirect->mHalf[kVifDirectImmediateHalf] += mCursor - mGifTag;
        }
    }
    if (bEop != 0) {
        pTag->mDword[kLowDword] |= kGifTagEop;
        mVifDirect = nullptr;
    }
    mGifTag = nullptr;
}

void DmaPacket::OpenGifTag(int nFlg, int nRegs, unsigned long long nRegList, int nPrim) {
    CloseGifTag(0, 0);
    if (mVif != 0 && mVifDirect == nullptr) {
        mVifDirect = mCursor;
        mCursor->mWord[kVifFirstWord] = kVifCodeFlushE;
        mCursor->mWord[1] = 0;
        mCursor->mWord[2] = 0;
        mCursor->mWord[kVifLastWord] = kVifCodeDirect;
        ++mCursor;
    }
    mGifTag = mCursor;
    unsigned long long nTag = static_cast<unsigned long long>(nFlg) << kGifTagFlgShift;
    if (nPrim != 0) {
        nTag |= kGifTagPre | (static_cast<unsigned long long>(nPrim) << kGifTagPrimShift);
    }
    nTag |= static_cast<unsigned long long>(nRegs) << kGifTagNregShift;
    mCursor->mDword[kLowDword] = nTag;
    mCursor->mDword[kHighDword] = nRegList;
    ++mCursor;
}

void DmaPacket::Init(DmaQuadword *pBase, int nChannel, int bVif) {
    mBase = pBase;
    mChannel = static_cast<unsigned short>(nChannel);
    mVif = static_cast<unsigned short>(bVif);
    Reset();
}

void DmaPacket::Reset() {
    mDmaTag = nullptr;
    mCursor = mBase;
    mGifTag = nullptr;
    mVifDirect = nullptr;
}

void DmaPacket::Wait() {
    while ((*kChcr[mChannel] & kChcrStart) != 0) {
    }
}

unsigned int DmaPacket::Close(Mode eMode, unsigned int nAddress) {
    const unsigned int nBase = DmaAddress(mBase);
    if (eMode == kModeNormal) {
        CloseGifTag(1, 0);
        Wait();
        *kQwc[mChannel] = static_cast<unsigned int>(mCursor - mBase);
        if (mChannel == kChannelFromSpr) {
            if (nAddress != 0) {
                *kMadr[mChannel] = nAddress;
            }
        } else {
            *kMadr[mChannel] = nBase;
        }
        if (mChannel == kChannelFromSpr) {
            *kSadr[mChannel] = nBase;
        } else if (mChannel == kChannelToSpr) {
            *kSadr[mChannel] = nAddress;
        }
        return kChcrStartNormal;
    }
    CloseDmaTag(kDmaTagIdEnd, 0, 0);
    Wait();
    *kQwc[mChannel] = 0;
    *kTadr[mChannel] = nBase;
    if (mChannel == kChannelToSpr) {
        *kSadr[mChannel] = nAddress;
    }
    return eMode != kModeChainTagged ? kChcrStartChain : kChcrStartChainTagged;
}

void DmaPacket::Send(Mode eMode, unsigned int nAddress) {
    const unsigned int nChcr = Close(eMode, nAddress);
    *kChcr[mChannel] = nChcr;
}
