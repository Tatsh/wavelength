#ifndef LIBKERNELINTERNAL_H
#define LIBKERNELINTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/** The system calls and data the kernel library uses without exporting them to the game. */

/** Sizes of the patch images the library copies into the kernel. */
enum {
    kAlarmPatchSize = 1856,    /*!< Size of g_alarmPatch in bytes. */
    kAlarmTrampolineSize = 40, /*!< Size of g_alarmTrampoline in bytes. */
    kExecPatchSize = 1960,     /*!< Size of g_execPatch in bytes. */
    kTlbPatchSize = 816,       /*!< Size of g_tlbPatch in bytes. */
};

/** The kernel system calls the patch installers use. */
enum {
    kSyscallKernelCopy = 90,      /*!< Copies memory in kernel mode, once a patch installs it. */
    kSyscallGetEntryAddress = 91, /*!< Reports a patch entry, once a patch installs it. */
    kSyscallSetSyscall = 116,     /*!< Installs the handler of a system call. */
};

/** One system call a patch installer installs. */
typedef struct {
    int nSyscall;   /*!< System call number. */
    void *pAddress; /*!< Handler, or null to install the entry the patch reports. */
} SyscallEntry;

/**
 * The alarm system calls, built to run at kernel address 0x80076000.
 *
 * @ghidraAddress NTSC-U/C: 0x003c5060
 * @ghidraAddress PAL: 0x0045b4a8
 */
extern const unsigned char g_alarmPatch[kAlarmPatchSize];

/**
 * The user-mode trampoline an alarm handler runs through, built to run at 0x00082000.
 *
 * @ghidraAddress NTSC-U/C: 0x003c57a0
 * @ghidraAddress PAL: 0x0045bbe8
 */
extern const unsigned char g_alarmTrampoline[kAlarmTrampolineSize];

/**
 * The LoadExecPS2() correction, built to run at kernel address 0x80074000.
 *
 * @ghidraAddress NTSC-U/C: 0x003c4898
 * @ghidraAddress PAL: 0x0045ace0
 */
extern const unsigned char g_execPatch[kExecPatchSize];

/**
 * The TLB system calls, built to run at kernel address 0x80075000.
 *
 * @ghidraAddress NTSC-U/C: 0x003c5808
 * @ghidraAddress PAL: 0x0045bc50
 */
extern const unsigned char g_tlbPatch[kTlbPatchSize];

/**
 * The kernel argument area LoadExecPS2() fills: the argument pointers, then the strings 64 bytes
 * in. InitTLBFunctions() records it.
 *
 * @ghidraAddress NTSC-U/C: 0x003c5b80
 * @ghidraAddress PAL: 0x0045bfc8
 */
extern char **g_ppExecArgs;

/**
 * The handler InitTLBFunctions() installs for system call 84. Runs in kernel mode, restores the
 * registers of the user thread from the save area, and returns to it.
 *
 * @ghidraAddress NTSC-U/C: 0x0031d1c0
 * @ghidraAddress PAL: 0x00389b00
 */
void HandleReturnFromExceptionToUserMode(void);

/**
 * Issues a kernel system call with up to five argument words. The kernel may change every
 * register the calling convention does not preserve.
 *
 * @param nNumber System call number. A negative number selects the interrupt form.
 * @param nFirst First argument word.
 * @param nSecond Second argument word.
 * @param nThird Third argument word.
 * @param nFourth Fourth argument word.
 * @param nFifth Fifth argument word.
 * @return The result word.
 */
static inline int
LibkernelSyscall5(int nNumber, int nFirst, int nSecond, int nThird, int nFourth, int nFifth) {
    register int v0 __asm__("$2");
    register int v1 __asm__("$3") = nNumber;
    register int a0 __asm__("$4") = nFirst;
    register int a1 __asm__("$5") = nSecond;
    register int a2 __asm__("$6") = nThird;
    register int a3 __asm__("$7") = nFourth;
    register int t0 __asm__("$8") = nFifth;

    __asm__ volatile(
        "syscall"
        : "=r"(v0), "+r"(v1), "+r"(a0), "+r"(a1), "+r"(a2), "+r"(a3), "+r"(t0)
        :
        : "$1", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "hi", "lo", "memory");
    return v0;
}

/**
 * Issues a kernel system call with up to three argument words.
 *
 * @param nNumber System call number. A negative number selects the interrupt form.
 * @param nFirst First argument word.
 * @param nSecond Second argument word.
 * @param nThird Third argument word.
 * @return The result word.
 */
static inline int LibkernelSyscall(int nNumber, int nFirst, int nSecond, int nThird) {
    return LibkernelSyscall5(nNumber, nFirst, nSecond, nThird, 0, 0);
}

/**
 * System call 20. Unmasks an interrupt cause.
 *
 * @param cause Interrupt cause.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x00316c60
 * @ghidraAddress PAL: 0x00383520
 */
int _EnableIntc(int cause);

/**
 * System call 21. Masks an interrupt cause.
 *
 * @param cause Interrupt cause.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x00316c70
 * @ghidraAddress PAL: 0x00383530
 */
int _DisableIntc(int cause);

/**
 * System call 22. Unmasks a DMA channel interrupt.
 *
 * @param channel DMA channel.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x00316c80
 * @ghidraAddress PAL: 0x00383540
 */
int _EnableDmac(int channel);

/**
 * System call 23. Masks a DMA channel interrupt.
 *
 * @param channel DMA channel.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x00316c90
 * @ghidraAddress PAL: 0x00383550
 */
int _DisableDmac(int channel);

/**
 * System call -52. Wakes a thread from an interrupt handler, other than the interrupted thread.
 *
 * @param thid Thread identifier.
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x00316e60
 * @ghidraAddress PAL: 0x00383720
 */
int _iWakeupThread(int thid);

#ifdef __cplusplus
}
#endif

#endif
