#include "ezmpeg/ldimage.h"

#include <stdint.h>

#include <eeregs.h>
#include <ezmpeg.h>

enum {
    kMacroblockSize = 16,
    kMacroblockBytes = kMacroblockSize * kMacroblockSize * 4,
    kMacroblockQwords = kMacroblockBytes / 16,
    // A DMA tag that sends the quadwords after it, one that sends a buffer elsewhere, and one that
    // ends the chain.
    kTagCnt = 1,
    kTagRef = 3,
    kTagEnd = 7,
    // The GIF tag register descriptor of address and data pairs, and the two GIF tag formats.
    kGifRegsAd = 0xe,
    kGifFlagPacked = 0,
    kGifFlagImage = 2,
    // GS registers of a host to local transfer.
    kGsRegBitbltbuf = 0x50,
    kGsRegTrxpos = 0x51,
    kGsRegTrxreg = 0x52,
    kGsRegTrxdir = 0x53,
    kFrameWidthPages = 10,
    kPsmCt32 = 0,
    kTransferHostToLocal = 0,
    // The quadwords each header tag sends.
    kHeaderQwords = 3,
    kBlockHeaderQwords = 4,
    kAdPairCount = 2,
    // The DMA control value that starts the GIF channel in chain mode from main memory.
    kGifChainStart = 0x105,
    kPhysicalAddressMask = 0x0fffffff,
    kUncachedSegment = 0x20000000,
};

#define UNCACHED(pointer)                                                                          \
    ((LoadImageTag *)(((uintptr_t)(pointer) & kPhysicalAddressMask) | kUncachedSegment))

// NTSC-U/C: 0x001b0700, PAL: 0x001b94a0
static void
setDMAscTag(LoadImageTag *tag, int spr, unsigned int addr, int irq, int id, int pce, int qwc) {
    tag->dword[0] = ((unsigned long long)spr << 63) |
                    ((unsigned long long)(addr & 0xfffffff0U) << 32) |
                    ((unsigned long long)(unsigned int)irq << 31) |
                    ((unsigned long long)(unsigned int)id << 28) |
                    ((unsigned long long)(unsigned int)pce << 26) | (unsigned int)qwc;
}

// NTSC-U/C: 0x001b0750, PAL: 0x001b94f0
static void setGIFtag(LoadImageTag *tag,
                      unsigned long long regs,
                      int nreg,
                      int flg,
                      int prim,
                      int pre,
                      int eop,
                      int nloop) {
    tag->word[0] = (unsigned int)((eop << 15) | nloop);
    tag->word[3] = (unsigned int)(regs >> 32);
    tag->word[1] = (unsigned int)((pre << 14) | (prim << 15) | (flg << 26) | (nreg << 28));
    tag->word[2] = (unsigned int)regs;
}

// NTSC-U/C: 0x001b07a0, PAL: 0x001b9540
static void setGIFad(LoadImageTag *tag, int addr, unsigned long long data) {
    tag->word[2] = (unsigned int)addr;
    tag->word[3] = 0;
    tag->word[0] = (unsigned int)data;
    tag->word[1] = (unsigned int)(data >> 32);
}

// NTSC-U/C: 0x001b07d0, PAL: 0x001b9570
static void setBITBLTBUF(LoadImageTag *tag, int dbp, int dbw, int dpsm) {
    setGIFad(tag,
             kGsRegBitbltbuf,
             ((unsigned long long)dpsm << 56) | ((unsigned long long)dbw << 48) |
                 ((unsigned long long)dbp << 32));
}

// NTSC-U/C: 0x001b0800, PAL: 0x001b95a0
static void setTRXPOS(LoadImageTag *tag, int dir, int dsax, int dsay) {
    setGIFad(tag,
             kGsRegTrxpos,
             ((unsigned long long)dir << 59) | ((unsigned long long)dsay << 48) |
                 ((unsigned long long)dsax << 32));
}

// NTSC-U/C: 0x001b0830, PAL: 0x001b95d0
static void setTRXREG(LoadImageTag *tag, int rrw, int rrh) {
    setGIFad(tag, kGsRegTrxreg, ((unsigned long long)rrh << 32) | (unsigned int)rrw);
}

// NTSC-U/C: 0x001b0860, PAL: 0x001b9600
static void setTRXDIR(LoadImageTag *tag, int xdir) {
    setGIFad(tag, kGsRegTrxdir, (unsigned int)xdir);
}

// NTSC-U/C: 0x001b0498, PAL: 0x001b9238
// Load the macroblocks column by column. Each macroblock follows the one before it by step bytes.
static void
setLoadImageTagsYX(LoadImageTag *tags, unsigned char *image, int step, int x, int y, int w, int h) {
    LoadImageTag *p = UNCACHED(tags);
    const int mbw = w >> 4;
    const int mbh = h >> 4;
    int i;

    setDMAscTag(p++, 0, 0, 0, kTagCnt, 0, kHeaderQwords);
    setGIFtag(p++, kGifRegsAd, 1, kGifFlagPacked, 0, 0, 0, kAdPairCount);
    setBITBLTBUF(p++, 0, kFrameWidthPages, kPsmCt32);
    setTRXREG(p++, kMacroblockSize, kMacroblockSize);
    for (i = 0; i < mbw; ++i) {
        int top = y;
        int j;
        for (j = 0; j < mbh; ++j) {
            const int eop = i == mbw - 1 && j == mbh - 1;
            setDMAscTag(p++, 0, 0, 0, kTagCnt, 0, kBlockHeaderQwords);
            setGIFtag(p++, kGifRegsAd, 1, kGifFlagPacked, 0, 0, 0, kAdPairCount);
            setTRXPOS(p++, 0, x + i * kMacroblockSize, top);
            top += kMacroblockSize;
            setTRXDIR(p++, kTransferHostToLocal);
            setGIFtag(p++, 0, 0, kGifFlagImage, 0, 0, eop, kMacroblockQwords);
            setDMAscTag(p++,
                        0,
                        (unsigned int)(uintptr_t)image & kPhysicalAddressMask,
                        0,
                        kTagRef,
                        0,
                        kMacroblockQwords);
            image += step;
        }
    }
    setDMAscTag(p, 0, 0, 0, kTagEnd, 0, 0);
}

void setLoadImageTags(LoadImageTag *tags, void *image, int x, int y, int w, int h) {
    setLoadImageTagsYX(tags, image, kMacroblockBytes, x, y, w, h);
}

void setLoadImageTagsTile(LoadImageTag *tags, void *image, int x, int y, int w, int h) {
    setLoadImageTagsYX(tags, image, 0, x, y, w, h);
}

void loadImage(LoadImageTag *tags) {
    *D2_TADR = (unsigned int)(uintptr_t)tags & kPhysicalAddressMask;
    *D2_QWC = 0;
    *D2_CHCR = kGifChainStart;
}
