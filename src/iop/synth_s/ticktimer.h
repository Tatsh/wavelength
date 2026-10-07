#pragma once

/**
 * The hardware timer that wakes the tick thread every six milliseconds.
 *
 * The class has no RTTI, and its name is inferred from its role. The module has one, #sTimer.
 */
class TickTimer {
public:
    /**
     * Allocate and configure the timer to wake #mThreadId.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00005e54
     * @ghidraAddress PAL: 0x00005e54
     */
    int Init();

    /**
     * Start the timer.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00005edc
     * @ghidraAddress PAL: 0x00005edc
     */
    int Start();

    /**
     * Release the timer.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00005f04
     * @ghidraAddress PAL: 0x00005f04
     */
    int Free();

    /**
     * Stop the timer.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00005f2c
     * @ghidraAddress PAL: 0x00005f2c
     */
    int Stop();

    /**
     * Create and start the tick thread, then start #sTimer to wake it.
     *
     * @param entry The tick thread's entry point.
     * @ghidraAddress NTSC-U/C: 0x00005f54
     * @ghidraAddress PAL: 0x00005f54
     */
    static void Begin(void (*entry)());

    /**
     * Stop and release #sTimer and delete the synthesiser's lock.
     *
     * @ghidraAddress NTSC-U/C: 0x00005fa8
     * @ghidraAddress PAL: 0x00005fa8
     */
    static void End();

    int mThreadId;         /*!< The thread each expiry wakes. */
    int mTimerId;          /*!< The hardware timer. */
    unsigned int mCompare; /*!< The period in system clock cycles. */

private:
    /**
     * Wake the timer's thread from interrupt context.
     *
     * @param timer The TickTimer.
     * @return The period. A nonzero period restarts the timer.
     * @ghidraAddress NTSC-U/C: 0x00005de8
     * @ghidraAddress PAL: 0x00005de8
     */
    static unsigned int Handler(void *timer);

    static TickTimer sTimer; /*!< The module's timer. */
    static int sThreadId;    /*!< The tick thread. */
};
