#pragma once

#include "gs/muse.h"

/**
 * Muse that sends one standard MIDI message.
 *
 * The RTTI includes the class name and records Muse as the base. This header declares only the
 * member a ScratchTrack uses.
 */
class StdMidiMuse : public Muse {
public:
    /**
     * Allocate a program change message.
     *
     * @param nChannel The MIDI channel.
     * @param nProgram The program number.
     * @return The message, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015c260
     * @ghidraAddress PAL: 0x0015da50
     */
    static StdMidiMuse *NewProgramChange(unsigned char nChannel, unsigned char nProgram);
};
