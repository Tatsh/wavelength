#ifndef KERNEL_H
#define KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IOP kernel services a module imports from the resident kernel libraries (loadcore, sysmem,
 * intrman, thbase, thevent, thsemap, timrman, and scrtpad). The layouts follow the stabs the
 * shipped EZMIDI.IRX records.
 */

/** Values a module entry returns to the loader. */
enum ModuleStartResult {
    RESIDENT_END = 0,           /*!< The module stays loaded. */
    NO_RESIDENT_END = 1,        /*!< The module does not stay loaded. */
    REMOVABLE_RESIDENT_END = 2, /*!< The module stays loaded and may be unloaded later. */
};

/** Module identity the loader reads through the `Module` symbol. */
typedef struct _moduleinfo {
    const char *name;       /*!< Module name. */
    unsigned short version; /*!< Major version in the high byte and minor in the low byte. */
} ModuleInfo;

/** Kernel result codes the module compares against. */
enum KernelErrorCode {
    KE_OK = 0,                  /*!< Success. */
    KE_ERROR = -1,              /*!< Unspecified failure. */
    KE_TIMER_NOT_INUSE = -156,  /*!< The timer is not running. */
    KE_ILLEGAL_PRIORITY = -413, /*!< The thread priority is out of range. */
};

/** Interrupt numbers of the IOP interrupt controller and its DMA channels. */
enum INUM {
    INUM_DMA_4 = 36, /*!< SPU2 core 0 DMA completion. */
    INUM_DMA_7 = 40, /*!< SPU2 core 1 DMA completion. */
};

/** Thread attribute for a thread written in C. */
#define TH_C 0x02000000

/** Creation parameters of a thread. */
struct ThreadParam {
    unsigned int attr;   /*!< Attribute bits such as #TH_C. */
    unsigned int option; /*!< Caller-defined option word. */
    union {
        void (*entry)(void);                       /*!< Entry point. */
        void (*entryWithArgument)(void *argument); /*!< Entry point that takes the StartThread()
                                                      argument. */
    };
    int stackSize;    /*!< Stack size in bytes. */
    int initPriority; /*!< Starting priority, where a smaller value runs first. */
};

/** A 64-bit system clock value in bus cycles. */
typedef struct {
    unsigned int low; /*!< Low word. */
    unsigned int hi;  /*!< High word. */
} SysClock;

/** Timer clock sources for AllocHardTimer() and SetupHardTimer(). */
#define TC_SYSCLOCK 1

/** Counter width a caller requests from AllocHardTimer(), in bits. */
#define TIMER_SIZE_32 32

/** Timer mode bits for SetupHardTimer(). */
#define TM_NO_GATE 0

/** Prescale divisor of one for AllocHardTimer() and SetupHardTimer(). */
#define TIMER_PRESCALE_1 1

/** Creation parameters of a semaphore. */
struct SemaParam {
    unsigned int attr;   /*!< Attribute bits. */
    unsigned int option; /*!< Caller-defined option word. */
    int initCount;       /*!< Starting count. */
    int maxCount;        /*!< Largest count. */
};

/**
 * Allocate IOP memory.
 *
 * @param mode Placement mode, zero for the first free block.
 * @param size Byte count.
 * @param address Requested address for an address placement mode, otherwise null.
 * @return The block, or null.
 */
void *AllocSysMemory(int mode, int size, void *address);

/**
 * Release IOP memory.
 *
 * @param block Block from AllocSysMemory().
 * @return #KE_OK, or a negative error code.
 */
int FreeSysMemory(void *block);

/**
 * Print formatted text to the kernel console.
 *
 * @param format Format string.
 * @return The number of characters printed.
 */
int Kprintf(const char *format, ...);

/**
 * Enable interrupts on the calling CPU.
 *
 * @return #KE_OK.
 */
int CpuEnableIntr(void);

/**
 * Disable interrupts on the calling CPU.
 *
 * @param state Receives the previous interrupt state.
 * @return #KE_OK, or a negative error code when interrupts were already disabled.
 */
int CpuSuspendIntr(int *state);

/**
 * Restore the interrupt state CpuSuspendIntr() saved.
 *
 * @param state The saved state.
 * @return #KE_OK.
 */
int CpuResumeIntr(int state);

/**
 * Unmask one interrupt source.
 *
 * @param intrcode Interrupt number, one of #INUM.
 * @return #KE_OK, or a negative error code.
 */
int EnableIntr(int intrcode);

/**
 * Create a dormant thread.
 *
 * @param param Creation parameters.
 * @return The thread identifier, or a negative error code.
 */
int CreateThread(struct ThreadParam *param);

/**
 * Delete a dormant thread.
 *
 * @param thid Thread identifier.
 * @return #KE_OK, or a negative error code.
 */
int DeleteThread(int thid);

/**
 * Stop another thread, leaving it dormant.
 *
 * @param thid Thread identifier.
 * @return #KE_OK, or a negative error code.
 */
int TerminateThread(int thid);

/**
 * Start a dormant thread.
 *
 * @param thid Thread identifier.
 * @param arg Argument the entry point receives.
 * @return #KE_OK, or a negative error code.
 */
int StartThread(int thid, void *arg);

/**
 * Change the priority of a thread.
 *
 * @param thid Thread identifier.
 * @param priority New priority, where a smaller value runs first.
 * @return #KE_OK, #KE_ILLEGAL_PRIORITY, or another negative error code.
 */
int ChangeThreadPriority(int thid, int priority);

/**
 * Identify the calling thread.
 *
 * @return The thread identifier.
 */
int GetThreadId(void);

/**
 * Sleep until another thread or a handler wakes the calling thread.
 *
 * @return #KE_OK, or a negative error code.
 */
int SleepThread(void);

/**
 * Wake a sleeping thread from thread context.
 *
 * @param thid Thread identifier.
 * @return #KE_OK, or a negative error code.
 */
int WakeupThread(int thid);

/**
 * Suspend the calling thread for a time.
 *
 * @param usec Microseconds.
 * @return #KE_OK, or a negative error code.
 */
int DelayThread(int usec);

/**
 * Wake a sleeping thread from interrupt context.
 *
 * @param thid Thread identifier.
 * @return #KE_OK, or a negative error code.
 */
int iWakeupThread(int thid);

/**
 * Create a semaphore.
 *
 * @param param Creation parameters.
 * @return The semaphore identifier, or a negative error code.
 */
int CreateSema(struct SemaParam *param);

/**
 * Delete a semaphore.
 *
 * @param semid Semaphore identifier.
 * @return #KE_OK, or a negative error code.
 */
int DeleteSema(int semid);

/**
 * Increment a semaphore, waking a waiting thread.
 *
 * @param semid Semaphore identifier.
 * @return #KE_OK, or a negative error code.
 */
int SignalSema(int semid);

/**
 * Decrement a semaphore, waiting while its count is zero.
 *
 * @param semid Semaphore identifier.
 * @return #KE_OK, or a negative error code.
 */
int WaitSema(int semid);

/**
 * Reserve the scratchpad memory.
 *
 * @param mode Reservation mode, zero.
 * @return The scratchpad address, or a negative error code in place of the address.
 */
void *AllocScratchPad(int mode);

/**
 * Release the scratchpad memory.
 *
 * @param address Address from AllocScratchPad().
 * @return #KE_OK, or a negative error code.
 */
int FreeScratchPad(void *address);

/**
 * Read the system clock.
 *
 * @param clock Receives the clock value.
 */
void GetSystemTime(SysClock *clock);

/**
 * Convert microseconds to system clock cycles.
 *
 * @param usec Microseconds.
 * @param clock Receives the cycle count.
 */
void USec2SysClock(unsigned int usec, SysClock *clock);

/**
 * Run a handler in interrupt context once a time has passed.
 *
 * @param clock Delay in system clock cycles.
 * @param handler Handler. It returns the next delay in cycles, or zero to stop.
 * @param common Argument the handler receives.
 * @return #KE_OK, or a negative error code.
 */
int SetAlarm(SysClock *clock, unsigned int (*handler)(void *common), void *common);

/**
 * Cancel an alarm that SetAlarm() installed.
 *
 * @param handler Handler of the alarm.
 * @param common Argument of the alarm.
 * @return #KE_OK, or a negative error code.
 */
int CancelAlarm(unsigned int (*handler)(void *common), void *common);

/** Creation parameters of an event flag. */
struct EventFlagParam {
    unsigned int attr;        /*!< Attribute bits. */
    unsigned int option;      /*!< Caller-defined option word. */
    unsigned int initPattern; /*!< Starting bit pattern. */
};

/** Wait mode bits for WaitEventFlag(). */
enum EventFlagWaitMode {
    WEF_OR = 0x01,    /*!< Wake when any requested bit is set, rather than all of them. */
    WEF_CLEAR = 0x10, /*!< Clear the requested bits on wake. */
};

/**
 * Create an event flag.
 *
 * @param param Creation parameters.
 * @return The event flag identifier, or a negative error code.
 */
int CreateEventFlag(struct EventFlagParam *param);

/**
 * Delete an event flag, waking its waiters with an error.
 *
 * @param evfid Event flag identifier.
 * @return #KE_OK, or a negative error code.
 */
int DeleteEventFlag(int evfid);

/**
 * Set bits of an event flag from thread context.
 *
 * @param evfid Event flag identifier.
 * @param bits Bits to set.
 * @return #KE_OK, or a negative error code.
 */
int SetEventFlag(int evfid, unsigned int bits);

/**
 * Set bits of an event flag from interrupt context.
 *
 * @param evfid Event flag identifier.
 * @param bits Bits to set.
 * @return #KE_OK, or a negative error code.
 */
int iSetEventFlag(int evfid, unsigned int bits);

/**
 * Wait until bits of an event flag are set.
 *
 * @param evfid Event flag identifier.
 * @param bits Bits to wait for.
 * @param mode Bits of #EventFlagWaitMode.
 * @param result Receives the bit pattern at wake.
 * @return #KE_OK, or a negative error code.
 */
int WaitEventFlag(int evfid, unsigned int bits, int mode, unsigned int *result);

/**
 * Allocate a hardware timer.
 *
 * @param source Clock source such as #TC_SYSCLOCK.
 * @param size Counter width in bits.
 * @param prescale Prescale divisor.
 * @return The timer identifier, or a negative error code.
 */
int AllocHardTimer(int source, int size, int prescale);

/**
 * Release a hardware timer.
 *
 * @param timid Timer identifier.
 * @return #KE_OK, or a negative error code.
 */
int FreeHardTimer(int timid);

/**
 * Install the compare handler of a hardware timer.
 *
 * @param timid Timer identifier.
 * @param compare Compare value in timer cycles.
 * @param handler Handler run in interrupt context. It returns the next compare value, or zero
 * to stop.
 * @param common Argument the handler receives.
 * @return #KE_OK, or a negative error code.
 */
int SetTimerHandler(int timid,
                    unsigned long compare,
                    unsigned int (*handler)(void *common),
                    void *common);

/**
 * Configure a hardware timer.
 *
 * @param timid Timer identifier.
 * @param source Clock source such as #TC_SYSCLOCK.
 * @param mode Mode bits such as #TM_NO_GATE.
 * @param prescale Prescale divisor.
 * @return #KE_OK, or a negative error code.
 */
int SetupHardTimer(int timid, int source, int mode, int prescale);

/**
 * Start a configured hardware timer.
 *
 * @param timid Timer identifier.
 * @return #KE_OK, or a negative error code.
 */
int StartHardTimer(int timid);

/**
 * Stop a running hardware timer.
 *
 * @param timid Timer identifier.
 * @return #KE_OK, or a negative error code.
 */
int StopHardTimer(int timid);

#ifdef __cplusplus
}
#endif

#endif
