#pragma once

#include "os/string.h"

/**
 * A sine wave the connector beam of a player's cursor sways and pulses with.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x20 bytes. The value at a
 * time is the sine of the frequency times 2 pi times the time, plus the phase.
 */
class TnlWave {
public:
    /**
     * Construct a wave with only a name.
     *
     * The amplitude, the frequency, and the phase remain indeterminate.
     *
     * @param pszName The name, or null.
     * @ghidraAddress NTSC-U/C: 0x001f96f0
     * @ghidraAddress PAL: 0x00202490
     */
    explicit TnlWave(const char *pszName);

    /**
     * Construct an unnamed wave.
     *
     * @param flAmplitude The amplitude.
     * @param flFrequency The frequency, in cycles per tick.
     * @param flPhase The phase in radians.
     * @ghidraAddress NTSC-U/C: 0x001f9718
     * @ghidraAddress PAL: 0x002024b8
     */
    TnlWave(float flAmplitude, float flFrequency, float flPhase);

    /**
     * Release the name.
     *
     * @ghidraAddress NTSC-U/C: 0x001f9778
     * @ghidraAddress PAL: 0x00202518
     */
    ~TnlWave();

    /**
     * Choose the phase that gives the wave a phase at a time.
     *
     * @param flTime The time.
     * @param flPhase The phase at that time.
     * @ghidraAddress NTSC-U/C: 0x001f97c0
     * @ghidraAddress PAL: 0x00202560
     */
    void Sync(float flTime, float flPhase);

    /**
     * Report the sine of the wave at a time.
     *
     * Every caller expands it.
     *
     * @param flTime The time.
     * @return The sine, from -1 to 1, before the amplitude applies.
     */
    float Sine(float flTime) const;

    String mName;     /*!< The name. */
    float mAmplitude; /*!< The amplitude. */
    float mFrequency; /*!< The frequency, in cycles per tick. */
    float mPhase;     /*!< The phase in radians. */
};
