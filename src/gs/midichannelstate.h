#pragma once

#include "gs/muse.h"

/**
 * Record of the controller, program, pressure, and pitch bend messages one MIDI channel has
 * received, from which a muse that restores the state is built.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Only the members the guitar
 * track builder uses are declared.
 */
class MidiChannelState {
public:
    /**
     * Construct the state of a channel with no message received.
     *
     * @param nChannel The channel.
     * @ghidraAddress NTSC-U/C: 0x001580b0
     * @ghidraAddress PAL: 0x00159938
     */
    explicit MidiChannelState(unsigned char nChannel);

    /**
     * Release the state.
     *
     * @ghidraAddress NTSC-U/C: 0x00158170
     * @ghidraAddress PAL: 0x001599f8
     */
    ~MidiChannelState();

    /**
     * Forget the received messages and follow another channel.
     *
     * @param nChannel The channel.
     * @ghidraAddress NTSC-U/C: 0x00158e08
     */
    void Reset(unsigned char nChannel);

    /**
     * Record a channel message, when it is a controller, program, pressure, or pitch bend message
     * of the channel.
     *
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00158228
     * @ghidraAddress PAL: 0x00159ab0
     */
    void Process(unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Build the muse that restores the recorded state.
     *
     * @return The muse, or null when no message was recorded.
     * @ghidraAddress NTSC-U/C: 0x00158640
     * @ghidraAddress PAL: 0x00159ec8
     */
    Muse *MakeMuse();
};
