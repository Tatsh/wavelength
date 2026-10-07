#pragma once

#include "gs/multimuse.h"

/**
 * Collector of MIDI channel messages that cuts MultiMuse pieces out of them.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. This header declares only
 * the members a ScratchTrackBuilder and a PitchTrackBuilder use.
 */
class MultiMuseBuilder {
public:
    /**
     * Construct an empty collector.
     *
     * @ghidraAddress NTSC-U/C: 0x0015a9f8
     * @ghidraAddress PAL: 0x0015c1e8
     */
    MultiMuseBuilder();

    /**
     * Release the collector.
     *
     * @ghidraAddress NTSC-U/C: 0x0015aa68
     * @ghidraAddress PAL: 0x0015c258
     */
    ~MultiMuseBuilder();

    /**
     * Collect a channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0015ab38
     * @ghidraAddress PAL: 0x0015c328
     */
    void AddMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Report the MIDI channel of the collected messages.
     *
     * @return The channel.
     * @ghidraAddress NTSC-U/C: 0x0015ac20
     * @ghidraAddress PAL: 0x0015c410
     */
    unsigned char GetChannel() const;

    /**
     * Count the collected messages in a span of ticks.
     *
     * A note-on message whose third word is negative does not count. The name is inferred.
     *
     * @param nStart The first tick of the span.
     * @param nEnd The tick after the span.
     * @return The number of messages.
     * @ghidraAddress NTSC-U/C: 0x0015ac28
     * @ghidraAddress PAL: 0x0015c418
     */
    int CountMessages(int nStart, int nEnd);

    /**
     * Build a piece from the messages in a span of ticks.
     *
     * @param nStart The first tick of the span.
     * @param nEnd The tick after the span.
     * @return The piece, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015ad98
     * @ghidraAddress PAL: 0x0015c588
     */
    MultiMuse *Extract(int nStart, int nEnd);
};
