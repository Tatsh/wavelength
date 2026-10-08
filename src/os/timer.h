#pragma once

#include <vector>

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
    /** Construct a stopped timer with no measurement. */
    Timer() {
        mRunning = 0;
        mCycles = 0;
        mLastMs = 0.0f;
    }

    /**
     * Stop the timer and record the cycles measured so far in milliseconds in mLastMs.
     */
    void Reset() {
        const float fMs = static_cast<float>(mCycles) * gSystemCycles2Ms;
        mRunning = 0;
        mCycles = 0;
        mLastMs = fMs;
    }

    /**
     * Suspend a running timer, adding the cycles since the start to mCycles. The start count
     * becomes negative until Resume().
     */
    void Pause() {
        if (mRunning > 0) {
            mRunning = -mRunning;
            mCycles += ReadCycleCount() - mStart;
        }
    }

    /**
     * Continue a timer Pause() suspended from the current counter reading.
     */
    void Resume() {
        if (mRunning < 0) {
            mRunning = -mRunning;
            mStart = ReadCycleCount();
        }
    }

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

    /**
     * Find a timer of the registry of named timers.
     *
     * @param pszName The name.
     * @return The timer, or null when no timer of the registry has the name.
     * @ghidraAddress NTSC-U/C: 0x0028d128
     * @ghidraAddress PAL: 0x00296b08
     */
    static Timer *Find(const char *pszName);

    /**
     * The registry of named timers.
     *
     * @ghidraAddress NTSC-U/C: 0x00491a00
     */
    static std::vector<Timer> sTimers;

    unsigned mStart;   /*!< Counter reading at the start of the current measurement. */
    unsigned mCycles;  /*!< Cycles of the last completed measurement. */
    float mLastMs;     /*!< mCycles converted to milliseconds when the timer last stopped. */
    const char *mName; /*!< The name Find() matches, or null for an unnamed timer. */
    int mRunning;      /*!< Start count. The timer measures while the count is non-zero. */
};
