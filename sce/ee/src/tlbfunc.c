#include <stddef.h>

#include <eekernel.h>
#include <libkernelinternal.h>

// The TLB system call installer, and the TLB layout of a console with 32 MB of memory. Each patch
// installer of the library has its own copies of the system call helpers.

enum {
    kSyscallReturnToUser = 84,
    kSyscallTlb85 = 85,
    kSyscallPutTLBEntry = 86,
    kSyscallTlb87 = 87,
    kSyscallTlb88 = 88,
    kSyscallTlb89 = 89,
    kTlbSyscallCount = 8,
    kTlbFirstLookup = 3,
    // The entry of the patch that reports the kernel argument area.
    kTlbEntryExecArgs = 3,

    kTlbEntryCount = 48,
    kTlbOverflowLimit = 49,
    kTlbKernelEntryCount = 13,
    kTlbDefaultEntryCount = 18,
    kTlbExtendedEntryCount = 8,
    kTlbUnmappedPageShift = 13,
    kTlbUnmappedPageStep = 1 << kTlbUnmappedPageShift,
    kMemorySize32MB = 0x2000000,
    kExitFailure = 1,
};

// The kernel address the TLB patch is copied to.
#define kTlbPatchBase ((void *)0x80075000)

// The virtual address the entries past the layout map, one 8 KB page each.
#define kTlbUnmappedBase 0xe0000000U

// One TLB entry as PutTLBEntry() writes it.
typedef struct {
    unsigned int nPageMask;
    unsigned int nEntryHi;
    unsigned int nEntryLo0;
    unsigned int nEntryLo1;
} TlbEntry;

// The TLB layout InitTLB32MB() writes. The wired entries are the kernel and default ones.
typedef struct {
    int nKernelCount;
    int nDefaultCount;
    int nExtendedCount;
    int nWired;
    const TlbEntry *pKernel;
    const TlbEntry *pDefault;
    const TlbEntry *pExtended;
} TlbLayout;

// NTSC-U/C: 0x003c5b80, PAL: 0x0045bfc8
char **g_ppExecArgs;

// NTSC-U/C: 0x0031ce00, PAL: 0x00389718
static __attribute__((noinline)) int Copy(void *pDest, const void *pSource, int nSize) {
    return LibkernelSyscall(kSyscallKernelCopy, (int)pDest, (int)pSource, nSize);
}

// NTSC-U/C: 0x0031ce10, PAL: 0x00389728
// The handler of system call 90 while the patch is copied. Copies whole words.
static int kCopy(unsigned int *pDest, const unsigned int *pSource, unsigned int nSize) {
    unsigned int nWords = nSize >> 2;
    unsigned int i;

    for (i = 0; i < nWords; ++i) {
        *pDest++ = *pSource++;
    }
    return 0;
}

// NTSC-U/C: 0x0031ce48, PAL: 0x00389760
static __attribute__((noinline)) void *GetEntryAddress(int nSyscall) {
    return (void *)LibkernelSyscall(kSyscallGetEntryAddress, nSyscall, 0, 0);
}

// NTSC-U/C: 0x0031ce58, PAL: 0x00389770
static __attribute__((noinline)) void setup(int nSyscall, void *pHandler) {
    LibkernelSyscall(kSyscallSetSyscall, nSyscall, (int)pHandler, 0);
}

// NTSC-U/C: 0x003c5b88, PAL: 0x0045bfd0
static SyscallEntry g_tlbSyscalls[kTlbSyscallCount] = {
    {kSyscallKernelCopy, kCopy},
    {kSyscallGetEntryAddress, kTlbPatchBase},
    {kSyscallReturnToUser, HandleReturnFromExceptionToUserMode},
    {kSyscallTlb85, NULL},
    {kSyscallPutTLBEntry, NULL},
    {kSyscallTlb87, NULL},
    {kSyscallTlb88, NULL},
    {kSyscallTlb89, NULL},
};

void InitTLBFunctions(void) {
    unsigned int i;

    setup(g_tlbSyscalls[0].nSyscall, g_tlbSyscalls[0].pAddress);
    Copy(kTlbPatchBase, g_tlbPatch, kTlbPatchSize);
    FlushCache(WRITEBACK_DCACHE);
    FlushCache(INVALIDATE_ICACHE);
    setup(g_tlbSyscalls[1].nSyscall, g_tlbSyscalls[1].pAddress);
    setup(g_tlbSyscalls[2].nSyscall, g_tlbSyscalls[2].pAddress);
    for (i = kTlbFirstLookup; i < kTlbSyscallCount; ++i) {
        setup(g_tlbSyscalls[i].nSyscall, GetEntryAddress(g_tlbSyscalls[i].nSyscall));
    }
    g_ppExecArgs = (char **)GetEntryAddress(kTlbEntryExecArgs);
}

// NTSC-U/C: 0x0031cf30, PAL: 0x00389848
// System call 86 once the patch installs it.
static __attribute__((noinline)) int PutTLBEntry(int nIndex,
                                                 unsigned int nPageMask,
                                                 unsigned int nEntryHi,
                                                 unsigned int nEntryLo0,
                                                 unsigned int nEntryLo1) {
    return LibkernelSyscall5(
        kSyscallPutTLBEntry, nIndex, (int)nPageMask, (int)nEntryHi, (int)nEntryLo0, (int)nEntryLo1);
}

// NTSC-U/C: 0x003c5bc8, the scratchpad, kernel, and register entries.
static const TlbEntry g_aTlbKernelEntries[kTlbKernelEntryCount] = {
    {0x00000000, 0x70000000, 0x80000007, 0x00000007},
    {0x00006000, 0xffff8000, 0x00001e1f, 0x00001f1f},
    {0x00000000, 0x10000000, 0x00400017, 0x00400053},
    {0x00000000, 0x10002000, 0x00400097, 0x004000d7},
    {0x00000000, 0x10004000, 0x00400117, 0x00400157},
    {0x00000000, 0x10006000, 0x00400197, 0x004001d7},
    {0x00000000, 0x10008000, 0x00400217, 0x00400257},
    {0x00000000, 0x1000a000, 0x00400297, 0x004002d7},
    {0x00000000, 0x1000c000, 0x00400313, 0x00400357},
    {0x00000000, 0x1000e000, 0x00400397, 0x004003d7},
    {0x0001e000, 0x11000000, 0x00440017, 0x00440415},
    {0x0001e000, 0x12000000, 0x00480017, 0x00480415},
    {0x01ffe000, 0x1e000000, 0x00780017, 0x007c0017},
};

// NTSC-U/C: 0x003c5c98, main memory through the cached and the uncached views.
static const TlbEntry g_aTlbDefaultEntries[kTlbDefaultEntryCount] = {
    {0x0007e000, 0x00080000, 0x0000201f, 0x0000301f},
    {0x0007e000, 0x00100000, 0x0000401f, 0x0000501f},
    {0x0007e000, 0x00180000, 0x0000601f, 0x0000701f},
    {0x001fe000, 0x00200000, 0x0000801f, 0x0000c01f},
    {0x001fe000, 0x00400000, 0x0001001f, 0x0001401f},
    {0x001fe000, 0x00600000, 0x0001801f, 0x0001c01f},
    {0x007fe000, 0x00800000, 0x0002001f, 0x0003001f},
    {0x007fe000, 0x01000000, 0x0004001f, 0x0005001f},
    {0x007fe000, 0x01800000, 0x0006001f, 0x0007001f},
    {0x0007e000, 0x20080000, 0x00002017, 0x00003017},
    {0x0007e000, 0x20100000, 0x00004017, 0x00005017},
    {0x0007e000, 0x20180000, 0x00006017, 0x00007017},
    {0x001fe000, 0x20200000, 0x00008017, 0x0000c017},
    {0x001fe000, 0x20400000, 0x00010017, 0x00014017},
    {0x001fe000, 0x20600000, 0x00018017, 0x0001c017},
    {0x007fe000, 0x20800000, 0x00020017, 0x00030017},
    {0x007fe000, 0x21000000, 0x00040017, 0x00050017},
    {0x007fe000, 0x21800000, 0x00060017, 0x00070017},
};

// NTSC-U/C: 0x003c5db8, main memory through the accelerated uncached view.
static const TlbEntry g_aTlbExtendedEntries[kTlbExtendedEntryCount] = {
    {0x0007e000, 0x30100000, 0x0000403f, 0x0000503f},
    {0x0007e000, 0x30180000, 0x0000603f, 0x0000703f},
    {0x001fe000, 0x30200000, 0x0000803f, 0x0000c03f},
    {0x001fe000, 0x30400000, 0x0001003f, 0x0001403f},
    {0x001fe000, 0x30600000, 0x0001803f, 0x0001c03f},
    {0x007fe000, 0x30800000, 0x0002003f, 0x0003003f},
    {0x007fe000, 0x31000000, 0x0004003f, 0x0005003f},
    {0x007fe000, 0x31800000, 0x0006003f, 0x0007003f},
};

// NTSC-U/C: 0x003c5e38, PAL: 0x0045c280
static TlbLayout g_tlbLayout = {
    kTlbKernelEntryCount,
    kTlbDefaultEntryCount,
    kTlbExtendedEntryCount,
    0,
    g_aTlbKernelEntries,
    g_aTlbDefaultEntries,
    g_aTlbExtendedEntries,
};

static inline void SetWired(int nWired) {
    __asm__ volatile("mtc0 %0, $6\n\tsync.p" : : "r"(nWired) : "memory");
}

// Writes nCount entries from the next free index, ending the program when they overflow the TLB.
static inline int
PutTLBEntries(int nIndex, const TlbEntry *pEntry, int nCount, const char *pszError) {
    int nEnd = nIndex + nCount;

    if (nEnd >= kTlbOverflowLimit) {
        kprintf(pszError);
        Exit(kExitFailure);
    }
    for (; nIndex < nEnd; ++nIndex) {
        PutTLBEntry(
            nIndex, pEntry->nPageMask, pEntry->nEntryHi, pEntry->nEntryLo0, pEntry->nEntryLo1);
        ++pEntry;
    }
    return nIndex;
}

// NTSC-U/C: 0x0031cf80, PAL: 0x00389898
// Returns the index past the layout shifted left by 13 bits. The binary shifts the count before
// it fills the remaining entries.
static int InitTLB32MB(void) {
    int nKernelEnd = g_tlbLayout.nKernelCount;
    int nDefaultEnd = nKernelEnd + g_tlbLayout.nDefaultCount;
    int nIndex;
    int nNext;
    unsigned int nUnmapped;

    kprintf("# TLB spad=0 kernel=1:%d default=%d:%d extended=%d:%d\n",
            nKernelEnd - 1,
            nKernelEnd,
            nDefaultEnd - 1,
            nDefaultEnd,
            nDefaultEnd + g_tlbLayout.nExtendedCount - 1);
    SetWired(0);
    nIndex = PutTLBEntries(0, g_tlbLayout.pKernel, g_tlbLayout.nKernelCount, "# TLB over flow (1)");
    nIndex = PutTLBEntries(
        nIndex, g_tlbLayout.pDefault, g_tlbLayout.nDefaultCount, "# TLB over flow (2)");
    g_tlbLayout.nWired = nIndex;
    SetWired(nIndex);
    if (g_tlbLayout.nExtendedCount > 0) {
        nIndex = PutTLBEntries(
            nIndex, g_tlbLayout.pExtended, g_tlbLayout.nExtendedCount, "# TLB over flow (3)");
    }
    nNext = nIndex;
    nIndex <<= kTlbUnmappedPageShift;
    for (nUnmapped = kTlbUnmappedBase + (unsigned int)nIndex; nNext < kTlbEntryCount; ++nNext) {
        PutTLBEntry(nNext, 0, nUnmapped, 0, 0);
        nUnmapped += kTlbUnmappedPageStep;
    }
    return nIndex;
}

int InitTLB(void) {
    if (GetMemorySize() == kMemorySize32MB) {
        return InitTLB32MB();
    }
    return _InitTLB();
}
