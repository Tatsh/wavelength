#include <stddef.h>

#include <eekernel.h>
#include <libkernelinternal.h>

// The LoadExecPS2() correction installer. Each patch installer of the library has its own copies
// of the system call helpers.

enum {
    // Bits 13 to 15 of the OSD configuration word. A kernel without the correction does not
    // retain them.
    kOsdConfigVersionShift = 13,
    kOsdConfigVersionMask = 7,
    kOsdConfigVersionClear = 0xffff1fff,
    kOsdConfigVersionProbe = 1 << kOsdConfigVersionShift,

    kExecSyscallCount = 3,
    kExecFirstLookup = 2,
};

// The kernel address the correction is copied to.
#define kExecPatchBase ((void *)0x80074000)

// NTSC-U/C: 0x0031c800, PAL: 0x00389118
static __attribute__((noinline)) void setup(int nSyscall, void *pHandler) {
    LibkernelSyscall(kSyscallSetSyscall, nSyscall, (int)pHandler, 0);
}

// NTSC-U/C: 0x0031c810, PAL: 0x00389128
static __attribute__((noinline)) int Copy(void *pDest, const void *pSource, int nSize) {
    return LibkernelSyscall(kSyscallKernelCopy, (int)pDest, (int)pSource, nSize);
}

// NTSC-U/C: 0x0031c820, PAL: 0x00389138
// The handler of system call 90 while the correction is copied. Copies whole words.
static int kCopy(unsigned int *pDest, const unsigned int *pSource, unsigned int nSize) {
    unsigned int nWords = nSize >> 2;
    unsigned int i;

    for (i = 0; i < nWords; ++i) {
        *pDest++ = *pSource++;
    }
    return 0;
}

// NTSC-U/C: 0x0031c858, PAL: 0x00389170
static __attribute__((noinline)) void *GetEntryAddress(int nSyscall) {
    return (void *)LibkernelSyscall(kSyscallGetEntryAddress, nSyscall, 0, 0);
}

// NTSC-U/C: 0x003c5040
static SyscallEntry g_execSyscalls[kExecSyscallCount] = {
    {kSyscallKernelCopy, kCopy},
    {kSyscallGetEntryAddress, kExecPatchBase},
    // Yes, the binary installs the correction under this number.
    {(int)0xffffc402, NULL},
};

// NTSC-U/C: 0x0031c868, PAL: 0x00389180
// Writes a probe value into the OSD configuration word, reads it back, and restores the word. A
// kernel that loses the probe needs the correction.
static int PatchIsNeeded(void) {
    unsigned int anConfig[2];

    GetOsdConfigParam(&anConfig[0]);
    anConfig[1] = (anConfig[0] & kOsdConfigVersionClear) | kOsdConfigVersionProbe;
    SetOsdConfigParam(&anConfig[1]);
    GetOsdConfigParam(&anConfig[1]);
    SetOsdConfigParam(&anConfig[0]);
    return ((anConfig[1] >> kOsdConfigVersionShift) & kOsdConfigVersionMask) == 0;
}

void InitExecPS2(void) {
    unsigned int i;

    if (!PatchIsNeeded()) {
        return;
    }
    setup(g_execSyscalls[0].nSyscall, g_execSyscalls[0].pAddress);
    Copy(kExecPatchBase, g_execPatch, kExecPatchSize);
    FlushCache(WRITEBACK_DCACHE);
    FlushCache(INVALIDATE_ICACHE);
    setup(g_execSyscalls[1].nSyscall, g_execSyscalls[1].pAddress);
    for (i = kExecFirstLookup; i < kExecSyscallCount; ++i) {
        setup(g_execSyscalls[i].nSyscall, GetEntryAddress(g_execSyscalls[i].nSyscall));
    }
}
