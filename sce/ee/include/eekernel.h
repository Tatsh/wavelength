#ifndef EEKERNEL_H
#define EEKERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Emotion Engine kernel services: threads, semaphores, interrupt and DMA handlers, caches, alarms,
 * and the GS and OSD configuration calls. Most entries are system calls, and the rest are library
 * routines built on them.
 */

/** Interrupt controller causes for AddIntcHandler() and EnableIntc(). */
enum {
    INTC_GS = 0,       /*!< GS interrupt. */
    INTC_SBUS = 1,     /*!< SBUS interrupt from the IOP. */
    INTC_VBLANK_S = 2, /*!< Start of vertical blank. */
    INTC_VBLANK_E = 3, /*!< End of vertical blank. */
    INTC_VIF0 = 4,     /*!< VIF0 interrupt. */
    INTC_VIF1 = 5,     /*!< VIF1 interrupt. */
    INTC_VU0 = 6,      /*!< VU0 interrupt. */
    INTC_VU1 = 7,      /*!< VU1 interrupt. */
    INTC_IPU = 8,      /*!< IPU interrupt. */
    INTC_TIM0 = 9,     /*!< Timer 0 interrupt. */
    INTC_TIM1 = 10,    /*!< Timer 1 interrupt. */
    INTC_TIM2 = 11,    /*!< Timer 2 interrupt. */
    INTC_TIM3 = 12,    /*!< Timer 3 interrupt. */
    INTC_SFIFO = 13,   /*!< SFIFO interrupt. */
    INTC_VU0WD = 14,   /*!< VU0 watchdog interrupt. */
};

/** DMA controller channels for AddDmacHandler() and EnableDmac(). */
enum {
    DMAC_VIF0 = 0,     /*!< VIF0 channel. */
    DMAC_VIF1 = 1,     /*!< VIF1 channel. */
    DMAC_GIF = 2,      /*!< GIF channel. */
    DMAC_FROM_IPU = 3, /*!< IPU output channel. */
    DMAC_TO_IPU = 4,   /*!< IPU input channel. */
    DMAC_SIF0 = 5,     /*!< SIF0 channel, IOP to EE. */
    DMAC_SIF1 = 6,     /*!< SIF1 channel, EE to IOP. */
    DMAC_SIF2 = 7,     /*!< SIF2 channel. */
    DMAC_FROM_SPR = 8, /*!< Scratchpad output channel. */
    DMAC_TO_SPR = 9,   /*!< Scratchpad input channel. */
    DMAC_CIS = 13,     /*!< Channel interrupt status. */
    DMAC_MEIS = 14,    /*!< Memory FIFO empty interrupt status. */
    DMAC_BEIS = 15,    /*!< Bus error interrupt status. */
};

/** Operations of FlushCache(). */
enum {
    WRITEBACK_DCACHE = 0,  /*!< Write the data cache back to memory. */
    INVALIDATE_DCACHE = 1, /*!< Discard the data cache. */
    INVALIDATE_ICACHE = 2, /*!< Discard the instruction cache. */
    INVALIDATE_CACHE = 3,  /*!< Discard both caches. */
};

/** Base of the uncached view of main memory. */
#define UNCACHED_SEG_BASE 0x20000000U

/** The uncached address of main memory at @p x. */
#define UNCACHED_SEG(x) ((void *)(((unsigned int)(x)) | UNCACHED_SEG_BASE))

/** Parameters and status of a thread. */
struct ThreadParam {
    int status;               /*!< Thread state bits. */
    void (*entry)(void *arg); /*!< Entry point. */
    void *stack;              /*!< Lowest address of the stack. */
    int stackSize;            /*!< Stack size in bytes. */
    void *gpReg;              /*!< Global pointer the thread runs with. */
    int initPriority;         /*!< Starting priority, where a smaller value runs first. */
    int currentPriority;      /*!< Current priority. */
    unsigned int attr;        /*!< Attribute bits. */
    unsigned int option;      /*!< Caller-defined option word. */
    int waitType;             /*!< What the thread waits on. */
    int waitId;               /*!< Semaphore the thread waits on. */
    int wakeupCount;          /*!< Pending wake-up requests. */
};

/** Parameters and status of a semaphore. */
struct SemaParam {
    int currentCount;    /*!< Current count. */
    int maxCount;        /*!< Largest count. */
    int initCount;       /*!< Starting count. */
    int numWaitThreads;  /*!< Threads waiting. */
    unsigned int attr;   /*!< Attribute bits. */
    unsigned int option; /*!< Caller-defined option word. */
};

/** Exits an interrupt handler. Synchronises memory and enables interrupts again. */
#define ExitHandler() __asm__ volatile("sync.l\n\tei")

/** The global pointer the linker assigns. */
extern char _gp[];

/**
 * Sets the GS display mode.
 *
 * @param interlace Non-zero for interlaced output.
 * @param omode Video mode (NTSC, PAL, or a progressive mode).
 * @param ffmd Non-zero to read a field at a time, zero to read a frame.
 * @ghidraAddress NTSC-U/C: 0x00316b20
 * @ghidraAddress PAL: 0x003833e0
 */
void SetGsCrt(short interlace, short omode, short ffmd);

/**
 * System call 4. Ends the program and returns to the system.
 *
 * @param status Exit status.
 * @ghidraAddress NTSC-U/C: 0x00316b40
 * @ghidraAddress PAL: 0x00383400
 */
void _Exit(int status);

/**
 * System call 6. Ends the program and starts another executable.
 *
 * @param filename The path of the executable.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @ghidraAddress NTSC-U/C: 0x00316b60
 * @ghidraAddress PAL: 0x00383420
 */
void _LoadExecPS2(const char *filename, int argc, char *argv[]);

/**
 * Restores the default TLB and ends the program through _Exit().
 *
 * @param status Exit status.
 * @ghidraAddress NTSC-U/C: 0x0031cb38
 * @ghidraAddress PAL: 0x00389450
 */
void Exit(int status);

/**
 * Copies the path and the arguments into the kernel argument area, restores the default TLB, and
 * starts another executable through _LoadExecPS2().
 *
 * @param filename The path of the executable.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @ghidraAddress NTSC-U/C: 0x0031caf0
 * @ghidraAddress PAL: 0x00389408
 */
void LoadExecPS2(const char *filename, int argc, char *argv[]);

/**
 * Adds an interrupt controller handler.
 *
 * @param cause Interrupt cause (`INTC_*`).
 * @param handler Handler.
 * @param next Position in the handler list (0 first, -1 last).
 * @return Handler identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x00316c00
 * @ghidraAddress PAL: 0x003834c0
 */
int AddIntcHandler(int cause, int (*handler)(int cause), int next);

/**
 * Removes an interrupt controller handler.
 *
 * @param cause Interrupt cause (`INTC_*`).
 * @param hid Handler identifier from AddIntcHandler().
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316c20
 * @ghidraAddress PAL: 0x003834e0
 */
int RemoveIntcHandler(int cause, int hid);

/**
 * Adds a DMA controller handler.
 *
 * @param channel DMA channel (`DMAC_*`).
 * @param handler Handler.
 * @param next Position in the handler list (0 first, -1 last).
 * @return Handler identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x00316c30
 * @ghidraAddress PAL: 0x003834f0
 */
int AddDmacHandler(int channel, int (*handler)(int channel), int next);

/**
 * Adds a DMA controller handler that also receives an argument.
 *
 * @param channel DMA channel (`DMAC_*`).
 * @param handler Handler, given the channel, @p arg, and the interrupted program counter.
 * @param next Position in the handler list (0 first, -1 last).
 * @param arg Argument for the handler.
 * @return Handler identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x00316c40
 * @ghidraAddress PAL: 0x00383500
 */
int AddDmacHandler2(int channel,
                    int (*handler)(int channel, void *arg, void *addr),
                    int next,
                    void *arg);

/**
 * Removes a DMA controller handler.
 *
 * @param channel DMA channel (`DMAC_*`).
 * @param hid Handler identifier from AddDmacHandler().
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316c50
 * @ghidraAddress PAL: 0x00383510
 */
int RemoveDmacHandler(int channel, int hid);

/**
 * Unmasks an interrupt cause. Interrupts are disabled around the system call.
 *
 * @param cause Interrupt cause (`INTC_*`).
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00317988
 * @ghidraAddress PAL: 0x003842a0
 */
int EnableIntc(int cause);

/**
 * Masks an interrupt cause. Interrupts are disabled around the system call.
 *
 * @param cause Interrupt cause (`INTC_*`).
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00317920
 * @ghidraAddress PAL: 0x00384238
 */
int DisableIntc(int cause);

/**
 * Unmasks a DMA channel interrupt. Interrupts are disabled around the system call.
 *
 * @param channel DMA channel (`DMAC_*`).
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00317a58
 * @ghidraAddress PAL: 0x00384370
 */
int EnableDmac(int channel);

/**
 * Masks a DMA channel interrupt. Interrupts are disabled around the system call.
 *
 * @param channel DMA channel (`DMAC_*`).
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x003179f0
 * @ghidraAddress PAL: 0x00384308
 */
int DisableDmac(int channel);

/**
 * Calls a handler after a delay. The handler runs in interrupt context.
 *
 * @param time Delay in horizontal blanks.
 * @param handler Handler, given the alarm identifier, the target time, and @p arg.
 * @param arg Argument for the handler.
 * @return Alarm identifier, or a negative value when every alarm is in use.
 * @ghidraAddress NTSC-U/C: 0x00316ca0
 * @ghidraAddress PAL: 0x00383560
 */
int SetAlarm(unsigned short time,
             void (*handler)(int id, unsigned short time, void *arg),
             void *arg);

/**
 * Calls a handler after a delay, from an interrupt handler.
 *
 * @param time Delay in horizontal blanks.
 * @param handler Handler, given the alarm identifier, the target time, and @p arg.
 * @param arg Argument for the handler.
 * @return Alarm identifier, or a negative value when every alarm is in use.
 * @ghidraAddress NTSC-U/C: 0x00316d00
 * @ghidraAddress PAL: 0x003835c0
 */
int iSetAlarm(unsigned short time,
              void (*handler)(int id, unsigned short time, void *arg),
              void *arg);

/**
 * Creates a thread in the dormant state.
 *
 * @param param Entry point, stack, global pointer, and priority.
 * @return Thread identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x00316d20
 * @ghidraAddress PAL: 0x003835e0
 */
int CreateThread(struct ThreadParam *param);

/**
 * Deletes a dormant thread.
 *
 * @param thid Thread identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316d30
 * @ghidraAddress PAL: 0x003835f0
 */
int DeleteThread(int thid);

/**
 * Starts a dormant thread.
 *
 * @param thid Thread identifier.
 * @param arg Argument for the entry point.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316d40
 * @ghidraAddress PAL: 0x00383600
 */
int StartThread(int thid, void *arg);

/**
 * Ends and deletes the calling thread. Does not return.
 *
 * @ghidraAddress NTSC-U/C: 0x00316d60
 * @ghidraAddress PAL: 0x00383620
 */
void ExitDeleteThread(void);

/**
 * Ends another thread and returns it to the dormant state.
 *
 * @param thid Thread identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316d70
 * @ghidraAddress PAL: 0x00383630
 */
int TerminateThread(int thid);

/**
 * Changes the priority of a thread.
 *
 * @param thid Thread identifier, or 0 for the calling thread.
 * @param priority New priority.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316db0
 * @ghidraAddress PAL: 0x00383670
 */
int ChangeThreadPriority(int thid, int priority);

/**
 * Moves the running thread of a priority to the end of its ready queue.
 *
 * @param priority Priority whose queue rotates.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316dd0
 * @ghidraAddress PAL: 0x00383690
 */
int RotateThreadReadyQueue(int priority);

/**
 * Returns the identifier of the calling thread.
 *
 * @ghidraAddress NTSC-U/C: 0x00316e10
 * @ghidraAddress PAL: 0x003836d0
 */
int GetThreadId(void);

/**
 * Reads the status of a thread.
 *
 * @param thid Thread identifier, or 0 for the calling thread.
 * @param info Receives the status.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316e20
 * @ghidraAddress PAL: 0x003836e0
 */
int ReferThreadStatus(int thid, struct ThreadParam *info);

/**
 * Puts the calling thread to sleep until a wake-up request arrives.
 *
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316e40
 * @ghidraAddress PAL: 0x00383700
 */
int SleepThread(void);

/**
 * Wakes a sleeping thread, or records the request when the thread is awake.
 *
 * @param thid Thread identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316e50
 * @ghidraAddress PAL: 0x00383710
 */
int WakeupThread(int thid);

/**
 * Wakes a sleeping thread from an interrupt handler. A request for the interrupted thread is
 * passed to a helper thread that InitThread() starts.
 *
 * @param thid Thread identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00317c70
 * @ghidraAddress PAL: 0x00384588
 */
int iWakeupThread(int thid);

/**
 * Suspends a thread.
 *
 * @param thid Thread identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316e90
 * @ghidraAddress PAL: 0x00383750
 */
int SuspendThread(int thid);

/**
 * Starts the helper thread that performs thread requests made from interrupt handlers, and raises
 * the calling thread to priority 1.
 *
 * @return Helper thread identifier, or -1 on failure or when the helper already runs.
 * @ghidraAddress NTSC-U/C: 0x00317b98
 * @ghidraAddress PAL: 0x003844b0
 */
int InitThread(void);

/**
 * Returns the end of the heap of the calling thread.
 *
 * @ghidraAddress NTSC-U/C: 0x00316f00
 * @ghidraAddress PAL: 0x003837c0
 */
void *EndOfHeap(void);

/**
 * Creates a semaphore.
 *
 * @param param Starting and largest counts.
 * @return Semaphore identifier, or a negative value on failure.
 * @ghidraAddress NTSC-U/C: 0x00316f20
 * @ghidraAddress PAL: 0x003837e0
 */
int CreateSema(struct SemaParam *param);

/**
 * Deletes a semaphore.
 *
 * @param semid Semaphore identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316f30
 * @ghidraAddress PAL: 0x003837f0
 */
int DeleteSema(int semid);

/**
 * Signals a semaphore.
 *
 * @param semid Semaphore identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316f40
 * @ghidraAddress PAL: 0x00383800
 */
int SignalSema(int semid);

/**
 * Signals a semaphore from an interrupt handler.
 *
 * @param semid Semaphore identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316f50
 * @ghidraAddress PAL: 0x00383810
 */
int iSignalSema(int semid);

/**
 * Waits on a semaphore.
 *
 * @param semid Semaphore identifier.
 * @return Non-negative on success, negative on failure.
 * @ghidraAddress NTSC-U/C: 0x00316f60
 * @ghidraAddress PAL: 0x00383820
 */
int WaitSema(int semid);

/**
 * Takes a semaphore without waiting.
 *
 * @param semid Semaphore identifier.
 * @return Non-negative when the semaphore was taken, negative otherwise.
 * @ghidraAddress NTSC-U/C: 0x00316f70
 * @ghidraAddress PAL: 0x00383830
 */
int PollSema(int semid);

/**
 * Reads the packed OSD configuration word (language, time zone, and screen settings).
 *
 * @param config Receives the configuration word.
 * @ghidraAddress NTSC-U/C: 0x00316fd0
 * @ghidraAddress PAL: 0x00383890
 */
void GetOsdConfigParam(unsigned int *config);

/**
 * Writes the packed OSD configuration word.
 *
 * @param config The configuration word.
 * @ghidraAddress NTSC-U/C: 0x00316fc0
 * @ghidraAddress PAL: 0x00383880
 */
void SetOsdConfigParam(unsigned int *config);

/**
 * Reads bytes of the extended OSD configuration.
 *
 * @param buffer Receives the bytes.
 * @param size Byte count.
 * @param offset Offset of the first byte.
 * @ghidraAddress NTSC-U/C: 0x00317230
 * @ghidraAddress PAL: 0x00383af0
 */
void GetOsdConfigParam2(void *buffer, int size, int offset);

/**
 * Writes back or discards the caches.
 *
 * @param operation Cache operation (`WRITEBACK_DCACHE`, `INVALIDATE_ICACHE`, and so on).
 * @ghidraAddress NTSC-U/C: 0x003171a0
 * @ghidraAddress PAL: 0x00383a60
 */
void FlushCache(int operation);

/**
 * Writes back the data cache lines of a memory range. Interrupts are disabled around the walk.
 *
 * @param start First address of the range.
 * @param end Last address of the range.
 * @ghidraAddress NTSC-U/C: 0x00317768
 * @ghidraAddress PAL: 0x00384080
 */
void SyncDCache(void *start, void *end);

/**
 * Discards the data cache lines of a memory range without writing them back. Interrupts are
 * disabled around the walk.
 *
 * @param start First address of the range.
 * @param end Last address of the range.
 * @ghidraAddress NTSC-U/C: 0x00317898
 * @ghidraAddress PAL: 0x003841b0
 */
void InvalidDCache(void *start, void *end);

/**
 * Returns the GS interrupt mask.
 *
 * @ghidraAddress NTSC-U/C: 0x00317240
 * @ghidraAddress PAL: 0x00383b00
 */
unsigned long long GsGetIMR(void);

/**
 * Sets the GS interrupt mask.
 *
 * @param imr New mask.
 * @return Previous mask.
 * @ghidraAddress NTSC-U/C: 0x00317260
 * @ghidraAddress PAL: 0x00383b20
 */
unsigned long long GsPutIMR(unsigned long long imr);

/**
 * Registers the words the kernel writes at the next vertical blank.
 *
 * @param flag Set to 1 at the vertical blank.
 * @param csr Receives the GS status register at the vertical blank.
 * @ghidraAddress NTSC-U/C: 0x00317290
 * @ghidraAddress PAL: 0x00383b50
 */
void SetVSyncFlag(unsigned int *flag, unsigned long long *csr);

/**
 * Installs the alarm system calls in the kernel. Does nothing when timer 3 already raises compare
 * interrupts.
 *
 * @ghidraAddress NTSC-U/C: 0x0031cbd0
 * @ghidraAddress PAL: 0x003894e8
 */
void InitAlarm(void);

/**
 * Installs the LoadExecPS2() correction in the kernel when the OSD configuration reports a kernel
 * that lacks it.
 *
 * @ghidraAddress NTSC-U/C: 0x0031c8d0
 * @ghidraAddress PAL: 0x003891e8
 */
void InitExecPS2(void);

/**
 * Installs the TLB system calls in the kernel and records the kernel argument area.
 *
 * @ghidraAddress NTSC-U/C: 0x0031ce68
 * @ghidraAddress PAL: 0x00389780
 */
void InitTLBFunctions(void);

/**
 * Restores the default TLB, through the 32 MB layout of the library on a console with 32 MB of
 * memory and through _InitTLB() otherwise.
 *
 * @return The number of entries the 32 MB layout wired, or the _InitTLB() result.
 * @ghidraAddress NTSC-U/C: 0x0031cf40
 * @ghidraAddress PAL: 0x00389858
 */
int InitTLB(void);

/**
 * System call 127. Returns the size of main memory in bytes.
 *
 * @ghidraAddress NTSC-U/C: 0x00317380
 * @ghidraAddress PAL: 0x00383c40
 */
int GetMemorySize(void);

/**
 * System call 130. Restores the TLB the kernel sets up at boot.
 *
 * @return Kernel result.
 * @ghidraAddress NTSC-U/C: 0x00317390
 * @ghidraAddress PAL: 0x00383c50
 */
int _InitTLB(void);

/**
 * Initialises the library at start-up. Creates the C library semaphores, finds the kernel system
 * call table, installs the alarm, LoadExecPS2(), and TLB system calls, and starts the helper
 * thread.
 *
 * @ghidraAddress NTSC-U/C: 0x0031c7c0
 * @ghidraAddress PAL: 0x003890d8
 */
void _InitSys(void);

/**
 * Disables interrupts.
 *
 * @return Non-zero when interrupts were enabled.
 * @ghidraAddress NTSC-U/C: 0x0031c578
 * @ghidraAddress PAL: 0x00388e90
 */
int DIntr(void);

/**
 * Enables interrupts.
 *
 * @return Non-zero when interrupts were already enabled.
 * @ghidraAddress NTSC-U/C: 0x0031c5c0
 * @ghidraAddress PAL: 0x00388ed8
 */
int EIntr(void);

/**
 * Calls a DECI2 service of the kernel.
 *
 * @param function Service number.
 * @param args Argument words.
 * @return Service result.
 * @ghidraAddress NTSC-U/C: 0x00317350
 * @ghidraAddress PAL: 0x00383c10
 */
int Deci2Call(int function, unsigned int *args);

/**
 * Writes a byte to the serial port once its transmit FIFO has room.
 *
 * @param c Byte.
 * @return @p c.
 * @ghidraAddress NTSC-U/C: 0x00318230
 * @ghidraAddress PAL: 0x00384b48
 */
int kputchar(int c);

/**
 * Formats text to the current console output, the serial port unless scePrintf() is running. The
 * format supports `%c`, `%s`, `%d`, `%u`, `%o`, `%x`, `%e`, and `%f`, the `h` and `l` size
 * prefixes, and a zero-padded width of up to 31 digits.
 *
 * @param format Format string.
 * @ghidraAddress NTSC-U/C: 0x00318b38
 * @ghidraAddress PAL: 0x00385450
 */
void kprintf(const char *format, ...);

/**
 * Formats text to the DECI2 kernel console. Output is sent a line at a time.
 *
 * @param format Format string, as kprintf() accepts.
 * @ghidraAddress NTSC-U/C: 0x00318b70
 * @ghidraAddress PAL: 0x00385488
 */
void scePrintf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
