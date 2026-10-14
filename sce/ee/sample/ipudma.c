#include "ezmpeg/ipudma.h"

#include <eeregs.h>
#include <libipu.h>

enum {
    kTagIdShift = 28,
    kTagAddrShift = 32,
    // A REF tag sends a buffer and continues the chain, and a REFE sends a buffer and ends it.
    kTagRef = 3,
    kTagRefe = 0,
    // The largest span one tag sends, a whole number of quadwords under the 16-bit count limit.
    kMaxSpanBytes = 0xffff0,
    kQwordBytes = 16,
    kPhysicalAddressMask = 0x0fffffff,
    // The DMA control value that starts the channel in chain mode from main memory.
    kIpuChainStart = 0x105,
    // FDEC with no bits to skip reads the next 32 bits, and FDEC 8 then skips one byte.
    kIpuFdec = 0x40000000,
    kIpuFdecByte = 0x40000008,
    kFirstByteShift = 24,
    kIpuSyncWait = 0,
};

void scTag(unsigned long long *tag, int addr, unsigned int id, unsigned int qwc) {
    *tag = ((unsigned long long)addr << kTagAddrShift) | ((unsigned long long)id << kTagIdShift) |
           (unsigned long long)qwc;
}

void setToIPUtags(unsigned long long *tags, unsigned int addr, int size) {
    addr &= kPhysicalAddressMask;
    while (size > 0) {
        const int span = size > kMaxSpanBytes ? kMaxSpanBytes : size;
        size -= span;
        scTag(tags,
              (int)addr,
              size != 0 ? kTagRef : kTagRefe,
              (unsigned int)((span + kQwordBytes - 1) / kQwordBytes));
        addr += span;
        tags += 2;
    }
}

void setDMAtoIPU(unsigned int tags) {
    *D4_TADR = tags & kPhysicalAddressMask;
    *D4_QWC = 0;
    *D4_CHCR = kIpuChainStart;
}

unsigned int getFlags(void) {
    unsigned int flags;
    *IPU_CMD = kIpuFdec;
    sceIpuSync(kIpuSyncWait, 0);
    flags = *IPU_CMD >> kFirstByteShift;
    sceIpuSync(kIpuSyncWait, 0);
    *IPU_CMD = kIpuFdecByte;
    sceIpuSync(kIpuSyncWait, 0);
    return flags;
}
