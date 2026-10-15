#include <stddef.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libkernelinternal.h>

// The alarm patch installer. Each patch installer of the library has its own copies of the system
// call helpers.

enum {
    kT3ModeCompareInterrupt = 0x100,

    kSyscallAlarmReturn = 8,
    kSyscallSetAlarm = 252,
    kSyscallReleaseAlarm = 253,
    kSyscallISetAlarm = 254,
    kSyscallIReleaseAlarm = 255,
    kSyscallAlarmInterrupt = 300,
    kAlarmSyscallCount = 8,
    kAlarmFirstLookup = 2,
};

// The kernel addresses the alarm patch and its handler trampoline are copied to.
#define kAlarmPatchBase ((void *)0x80076000)
#define kAlarmHandlerTrampoline ((void *)0x00082000)

// NTSC-U/C: 0x0031cb68, PAL: 0x00389480
static __attribute__((noinline)) void setup(int nSyscall, void *pHandler) {
    LibkernelSyscall(kSyscallSetSyscall, nSyscall, (int)pHandler, 0);
}

// NTSC-U/C: 0x0031cb78, PAL: 0x00389490
static __attribute__((noinline)) int Copy(void *pDest, const void *pSource, int nSize) {
    return LibkernelSyscall(kSyscallKernelCopy, (int)pDest, (int)pSource, nSize);
}

// NTSC-U/C: 0x0031cb88, PAL: 0x003894a0
// The handler of system call 90 while the patch is copied. Copies whole words.
static int kCopy(unsigned int *pDest, const unsigned int *pSource, unsigned int nSize) {
    unsigned int nWords = nSize >> 2;
    unsigned int i;

    for (i = 0; i < nWords; ++i) {
        *pDest++ = *pSource++;
    }
    return 0;
}

// NTSC-U/C: 0x0031cbc0, PAL: 0x003894d8
static __attribute__((noinline)) void *GetEntryAddress(int nSyscall) {
    return (void *)LibkernelSyscall(kSyscallGetEntryAddress, nSyscall, 0, 0);
}

// NTSC-U/C: 0x003c57c8, PAL: 0x0045bc10
static SyscallEntry g_alarmSyscalls[kAlarmSyscallCount] = {
    {kSyscallKernelCopy, kCopy},
    {kSyscallGetEntryAddress, kAlarmPatchBase},
    {kSyscallSetAlarm, NULL},
    {kSyscallISetAlarm, NULL},
    {kSyscallReleaseAlarm, NULL},
    {kSyscallIReleaseAlarm, NULL},
    {kSyscallAlarmInterrupt, NULL},
    {kSyscallAlarmReturn, NULL},
};

void InitAlarm(void) {
    int i;

    if ((*T3_MODE & kT3ModeCompareInterrupt) != 0) {
        return;
    }
    setup(g_alarmSyscalls[0].nSyscall, g_alarmSyscalls[0].pAddress);
    Copy(kAlarmPatchBase, g_alarmPatch, kAlarmPatchSize);
    Copy(kAlarmHandlerTrampoline, g_alarmTrampoline, kAlarmTrampolineSize);
    FlushCache(WRITEBACK_DCACHE);
    FlushCache(INVALIDATE_ICACHE);
    setup(g_alarmSyscalls[1].nSyscall, g_alarmSyscalls[1].pAddress);
    for (i = kAlarmFirstLookup; i < kAlarmSyscallCount; ++i) {
        setup(g_alarmSyscalls[i].nSyscall, GetEntryAddress(g_alarmSyscalls[i].nSyscall));
    }
}
