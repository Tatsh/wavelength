#include <stddef.h>

#include <eekernel.h>
#include <libkernelinternal.h>

// The library start-up, and the search for the kernel system call table.

enum {
    kSyscallFindAddress = 131,
    kInitSysSyscallCount = 2,
};

// The kernel memory the search for the system call table covers.
#define kKernelSearchStart ((unsigned int *)0x80000000)
#define kKernelSearchEnd ((unsigned int *)0x80080000)

// NTSC-U/C: 0x003c4878, the kernel system call table. The library records it and never reads it.
static void **g_ppSyscallTable;

// NTSC-U/C: 0x003c4890
static int g_anLibcSemaphores[2];

// NTSC-U/C: 0x0031c5d8, PAL: 0x00388ef0
static void InitLibcSemas(void) {
    struct SemaParam first;
    struct SemaParam second;

    first.maxCount = 1;
    first.initCount = 1;
    second.maxCount = 1;
    second.initCount = 1;
    g_anLibcSemaphores[0] = CreateSema(&first);
    g_anLibcSemaphores[1] = CreateSema(&second);
}

// NTSC-U/C: 0x0031c620, PAL: 0x00388f38
// The handler of system call 90 during the search. Copies whole words.
static int kCopy(unsigned int *pDest, const unsigned int *pSource, unsigned int nSize) {
    unsigned int nWords = nSize >> 2;
    unsigned int i;

    for (i = 0; i < nWords; ++i) {
        *pDest++ = *pSource++;
    }
    return 0;
}

// NTSC-U/C: 0x0031c658, PAL: 0x00388f70
// The handler of system call 131. Returns the first word from pStart that equals nValue, or null
// when the word is found at or past pEnd.
static unsigned int *kFindAddress(unsigned int *pStart, unsigned int *pEnd, unsigned int nValue) {
    unsigned int *p = pStart;

    for (;;) {
        if (*p == nValue) {
            return p < pEnd ? p : NULL;
        }
        if (!(p < pEnd)) {
            return NULL;
        }
        ++p;
    }
}

// NTSC-U/C: 0x0031c698, PAL: 0x00388fb0
static __attribute__((noinline)) unsigned int *
FindAddress(unsigned int *pStart, unsigned int *pEnd, void *pValue) {
    return (unsigned int *)LibkernelSyscall(
        kSyscallFindAddress, (int)pStart, (int)pEnd, (int)pValue);
}

// NTSC-U/C: 0x0031c7b0, PAL: 0x003890c8
static __attribute__((noinline)) void setup(int nSyscall, void *pHandler) {
    LibkernelSyscall(kSyscallSetSyscall, nSyscall, (int)pHandler, 0);
}

// NTSC-U/C: 0x003c4880
static SyscallEntry g_initSysSyscalls[kInitSysSyscallCount] = {
    {kSyscallFindAddress, kFindAddress},
    {kSyscallKernelCopy, kCopy},
};

// NTSC-U/C: 0x0031c6b0, PAL: 0x00388fc8
// Installs two handlers and finds the table entries that hold them. The table starts where both
// entries agree.
static void InitSystemCallTableAddress(void) {
    unsigned int *pFindEntry;
    unsigned int *pCopyEntry;
    unsigned int *pFromFind;
    unsigned int *pFromCopy;

    setup(g_initSysSyscalls[0].nSyscall, g_initSysSyscalls[0].pAddress);
    setup(g_initSysSyscalls[1].nSyscall, g_initSysSyscalls[1].pAddress);
    pFindEntry = FindAddress(kKernelSearchStart, kKernelSearchEnd, kFindAddress);
    pCopyEntry = FindAddress(kKernelSearchStart, kKernelSearchEnd, kCopy);
    pFromFind = pFindEntry - kSyscallFindAddress;
    pFromCopy = pCopyEntry - kSyscallKernelCopy;
    while (pFromFind != pFromCopy) {
        if (pFromFind < pFromCopy) {
            pFindEntry = FindAddress(pFindEntry + 1, kKernelSearchEnd, kFindAddress);
            pFromFind = pFindEntry - kSyscallFindAddress;
        } else {
            pCopyEntry = FindAddress(pCopyEntry + 1, kKernelSearchEnd, kCopy);
            pFromCopy = pCopyEntry - kSyscallKernelCopy;
        }
    }
    g_ppSyscallTable = (void **)pFromFind;
}

void _InitSys(void) {
    InitLibcSemas();
    InitSystemCallTableAddress();
    InitAlarm();
    (void)InitThread();
    InitExecPS2();
    InitTLBFunctions();
}
