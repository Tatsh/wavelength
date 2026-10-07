#pragma once

#include "os/cycles.h"

/**
 * Stopwatch on the EE cycle counter.
 *
 * The RTTI includes the class name. The class is not polymorphic, and the object is 0x14 bytes
 * with no vptr. The system clock at `0x00491a10` is one instance, and many routines build another
 * on the stack to time their body.
 *
 * Every member routine in the image is expanded inline at its call sites.
 */
class Timer {
public:
    /**
     * Restart the measurement at the current counter reading.
     *
     * Only a timer whose mRunning is exactly 1 is split. Any other timer is unchanged. A split
     * timer stores the cycles since the previous reading in mCycles, replacing the earlier value.
     */
    void Split() {
        if (mRunning == 1) {
            const unsigned nCount = ReadCycleCount();
            mCycles = nCount - mStart;
            mStart = nCount;
        }
    }

    /**
     * Raise the start count, and take the counter reading when the count leaves zero.
     */
    void Start() {
        if (mRunning++ == 0) {
            mStart = ReadCycleCount();
        }
    }

    /**
     * Lower the start count, and add the cycles since the start to mCycles when it arrives at zero.
     */
    void Stop() {
        if (--mRunning == 0) {
            mCycles += ReadCycleCount() - mStart;
        }
    }

    unsigned mStart;    /*!< Counter reading at the start of the current measurement. */
    unsigned mCycles;   /*!< Cycles of the last completed measurement. */
    float mLastMs;      /*!< mCycles converted to milliseconds when the timer last stopped. */
    unsigned mReserved; // +0x0c, not written by any routine recovered so far.
    int mRunning;       /*!< Start count. The timer measures while the count is non-zero. */
};
