#pragma once

/**
 * A hardware timer that wakes one thread at a fixed interval.
 *
 * The class is not polymorphic and emits no RTTI. Its name is inferred from its routines.
 */
class PeriodicTimer {
public:
    /**
     * Wake the timer's thread. The timer runs it in interrupt context.
     *
     * @param common The PeriodicTimer.
     * @return The interval, which schedules the next compare.
     * @ghidraAddress NTSC-U/C: 0x00000e08
     * @ghidraAddress PAL: 0x00000e08
     */
    static unsigned int Handler(void *common);

    /**
     * Allocate a 32-bit system clock timer with Handler() at a 6 millisecond interval.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000e74
     * @ghidraAddress PAL: 0x00000e74
     */
    int Init();

    /**
     * Start the timer.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000efc
     * @ghidraAddress PAL: 0x00000efc
     */
    int Start();

    /**
     * Release the timer.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000f24
     * @ghidraAddress PAL: 0x00000f24
     */
    int Free();

    /**
     * Stop the timer.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000f4c
     * @ghidraAddress PAL: 0x00000f4c
     */
    int Stop();

    int mThread;            /*!< Thread the timer wakes. */
    int mTimer;             /*!< Hardware timer identifier. */
    unsigned int mInterval; /*!< Interval in system clock cycles. */
};

/**
 * Create a dormant C thread with a 2 KiB stack.
 *
 * @param entry Entry point.
 * @param priority Starting priority.
 * @return The thread identifier, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x00000e38
 * @ghidraAddress PAL: 0x00000e38
 */
int CreateThreadWithPriority(void (*entry)(), int priority);

/**
 * Start a thread the tick timer wakes every 6 milliseconds. The routine has no caller.
 *
 * @param entry Entry point of the thread.
 * @ghidraAddress NTSC-U/C: 0x00000f74
 * @ghidraAddress PAL: 0x00000f74
 */
void StartTickThread(void (*entry)());

/**
 * Stop and release the tick timer, and delete the lock. The routine has no caller.
 *
 * @ghidraAddress NTSC-U/C: 0x00000fc8
 * @ghidraAddress PAL: 0x00000fc8
 */
void StopTickThread();
