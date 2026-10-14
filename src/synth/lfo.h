#pragma once

#include "os/timer.h"

/**
 * Low-frequency oscillator that runs on the EE cycle counter.
 *
 * The class has no RTTI and is not polymorphic, and the name is inferred. The object is 0x24
 * bytes. SynthPS2 modulates the delay of each tap of the chorus of the software effect with one.
 */
class Lfo {
public:
    /** Shapes of the oscillation, the values of mWaveform. */
    enum Waveform {
        kWaveformSine = 0,     /*!< A sine wave from -1 to 1. */
        kWaveformTriangle = 1, /*!< A triangle wave from -1 to 1 that starts at 0 rising. */
        kWaveformSquare = 2,   /*!< 1 for the first half of each cycle and -1 for the second. */
        kWaveformConstant = 3, /*!< A constant 1. */
    };

    /**
     * Construct a stopped sine oscillator with a period of one second.
     *
     * mElapsedMs is not initialised.
     *
     * @ghidraAddress NTSC-U/C: 0x00251e20
     * @ghidraAddress PAL: 0x0025a7c8
     */
    Lfo();

    /**
     * Start the clock of the oscillator.
     *
     * @ghidraAddress NTSC-U/C: 0x00251e48
     * @ghidraAddress PAL: 0x0025a7f0
     */
    void Start();

    /**
     * Stop the clock of the oscillator.
     *
     * The time already accumulated in mElapsedMs is unchanged.
     *
     * @ghidraAddress NTSC-U/C: 0x00251e78
     * @ghidraAddress PAL: 0x0025a820
     */
    void Stop();

    /**
     * Add the time since the last call to the elapsed time and restart the clock.
     *
     * The elapsed time wraps to stay within one period.
     *
     * @return The elapsed time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00251ef8
     * @ghidraAddress PAL: 0x0025a8a0
     */
    float Tick();

    /**
     * Advance the clock and report the value of the oscillation.
     *
     * @return The value, from -1 to 1.
     * @ghidraAddress NTSC-U/C: 0x00252020
     * @ghidraAddress PAL: 0x0025a9c8
     */
    float Evaluate();

    /**
     * Convert a time to cycles of the oscillation.
     *
     * @param fSeconds The time in seconds.
     * @return The number of cycles.
     * @ghidraAddress NTSC-U/C: 0x00252060
     * @ghidraAddress PAL: 0x0025aa08
     */
    float GetCycles(float fSeconds) const;

    /**
     * Report the value of the oscillation at a time.
     *
     * @param fSeconds The time in seconds.
     * @return The value, from -1 to 1, or 0 for an unknown waveform.
     * @ghidraAddress NTSC-U/C: 0x00252078
     * @ghidraAddress PAL: 0x0025aa20
     */
    float ValueAt(float fSeconds) const;

    Timer mTimer;     /*!< The clock of the oscillator. */
    int mWaveform;    /*!< One of Waveform. */
    float mPeriod;    /*!< The period in seconds. */
    float mPhase;     /*!< The offset of the start in whole cycles. */
    float mElapsedMs; /*!< The time accumulated within the current period, in milliseconds. */
};
