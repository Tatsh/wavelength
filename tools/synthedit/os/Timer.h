#pragma once

/**
 * The time-stamp counter at the last reading.
 *
 * @ghidraAddress 0x10040c98
 */
extern unsigned int gLastCycleCount;

/**
 * The cycles between the last two readings.
 *
 * @ghidraAddress 0x10040c9c
 */
extern unsigned int gDeltaCycles;

/**
 * 1 once the counter is calibrated and read at every update.
 *
 * @ghidraAddress 0x10040ca8
 */
extern int gCalibrateCount;

/**
 * The milliseconds of one cycle of the time-stamp counter.
 *
 * @ghidraAddress 0x10040cb0
 */
extern float gMsPerCycle;

/**
 * The cycles counted since the timer started.
 *
 * @ghidraAddress 0x10040cb8
 */
extern unsigned __int64 gTotalCycles;

/** @return The low word of the processor's time-stamp counter. */
inline unsigned int ReadTimeStampCounter() {
    unsigned int low;
    __asm {
        push edx
        push eax
        _emit 0x0f // rdtsc
        _emit 0x31
        mov low, eax
        pop eax
        pop edx
    }
    return low;
}

/**
 * Add the cycles since the last reading to the total.
 *
 * @return The time since the timer started, in milliseconds.
 */
inline float TimerUpdateMs() {
    if (gCalibrateCount == 1) {
        const unsigned int cycles = ReadTimeStampCounter();
        gDeltaCycles = cycles - gLastCycleCount;
        gLastCycleCount = cycles;
    }
    gTotalCycles += gDeltaCycles;
    return static_cast<float>(static_cast<__int64>(gTotalCycles)) * gMsPerCycle;
}

/**
 * A named timer of the `timer` configuration.
 *
 * The RTTI records the class. The object is 0x14 bytes. Only the name is read by the control; the
 * other members are placeholders that record the layout.
 */
class Timer {
public:
    Timer() : mReserved04(0), mReserved08(0), mReserved10(0) {
    }

    int mReserved00;   // +0x00, not set by the constructor and not yet identified.
    int mReserved04;   // +0x04, cleared by the constructor and not yet identified.
    int mReserved08;   // +0x08, cleared by the constructor and not yet identified.
    const char *mName; /*!< The name, from the configuration. +0x0c */
    int mReserved10;   // +0x10, cleared by the constructor and not yet identified.
};
