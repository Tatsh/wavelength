#pragma once

#include "os/string.h"

/**
 * Song position split into a measure, a beat, and a tick within the beat.
 *
 * The class is not polymorphic and has no RTTI. The name follows the `Mid::MBT` of the related
 * engine. The error messages of the track builders print the position of a MIDI event with it.
 */
class MBT {
public:
    /**
     * Split a position.
     *
     * @param nTick The position in ticks.
     * @param nBeatsPerMeasure The beats in one measure.
     * @param nTicksPerBeat The ticks in one beat.
     * @ghidraAddress NTSC-U/C: 0x0015c378
     * @ghidraAddress PAL: 0x0015db68
     */
    MBT(int nTick, int nBeatsPerMeasure, int nTicksPerBeat);

    /**
     * Format the position as `measure:beat:tick`.
     *
     * @return The text.
     * @ghidraAddress NTSC-U/C: 0x0015c3b8
     * @ghidraAddress PAL: 0x0015dba8
     */
    String ToString() const;

    int mMeasure; /*!< The measure, counted from 1. */
    int mBeat;    /*!< The beat within the measure, counted from 1. */
    int mTick;    /*!< The tick within the beat, counted from 0. */
};
