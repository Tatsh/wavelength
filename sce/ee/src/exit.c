#include <string.h>

#include <eekernel.h>
#include <libkernelinternal.h>

// Ending the program, and starting another executable with arguments the kernel keeps.

enum {
    kExecArgMax = 16,
    // The strings follow the argument pointers in the kernel argument area.
    kExecStringOffset = 64,
};

// NTSC-U/C: 0x0031c980
static __attribute__((noinline)) int Copy(void *pDest, const void *pSource, int nSize) {
    return LibkernelSyscall(kSyscallKernelCopy, (int)pDest, (int)pSource, nSize);
}

// NTSC-U/C: 0x0031c990, PAL: 0x00389298
// The handler of system call 90 while the arguments are copied. Copies bytes.
static int kCopy(unsigned char *pDest, const unsigned char *pSource, unsigned int nSize) {
    unsigned int i;

    for (i = 0; i < nSize; ++i) {
        *pDest++ = *pSource++;
    }
    return 0;
}

// NTSC-U/C: 0x0031c9c0, PAL: 0x003892d8
static __attribute__((noinline)) void setup(int nSyscall, void *pHandler) {
    LibkernelSyscall(kSyscallSetSyscall, nSyscall, (int)pHandler, 0);
}

// NTSC-U/C: 0x003c5058
static SyscallEntry g_exitCopySyscall = {kSyscallKernelCopy, kCopy};

// NTSC-U/C: 0x0031c9d0, PAL: 0x003892e8
// Copies the path and at most 15 arguments into the kernel argument area. Returns the copy of the
// path.
static char *SetArg(const char *filename, int argc, char *argv[]) {
    char **ppArgs = g_ppExecArgs;
    char *pStrings = (char *)ppArgs + kExecStringOffset;
    char *pPath = pStrings;
    int nLength;

    setup(g_exitCopySyscall.nSyscall, g_exitCopySyscall.pAddress);
    if (argc >= kExecArgMax) {
        argc = kExecArgMax - 1;
    }
    Copy(ppArgs, &pStrings, sizeof(pStrings));
    ++ppArgs;
    nLength = (int)strlen(filename) + 1;
    Copy(pStrings, filename, nLength);
    pStrings += nLength;
    for (; argc > 0; --argc) {
        Copy(ppArgs, &pStrings, sizeof(pStrings));
        ++ppArgs;
        nLength = (int)strlen(*argv) + 1;
        Copy(pStrings, *argv, nLength);
        ++argv;
        pStrings += nLength;
    }
    return pPath;
}

// NTSC-U/C: 0x0031cae8, PAL: 0x00389400
static void TerminateLibrary(void) {
    InitTLB();
}

void LoadExecPS2(const char *filename, int argc, char *argv[]) {
    char *pPath = SetArg(filename, argc, argv);

    TerminateLibrary();
    // Yes, the binary passes the count before SetArg() limits it.
    _LoadExecPS2(pPath, argc, g_ppExecArgs + 1);
}

void Exit(int status) {
    TerminateLibrary();
    _Exit(status);
}
