#pragma once

#include <vector>

#include "gs/midichannelstate.h"
#include "gs/multimuse.h"
#include "gs/muse.h"

/**
 * Collector of the channel messages of one MIDI channel that cuts MultiMuse pieces out of them.
 *
 * The class is not polymorphic. The RTTI records the name through the nested MuseFactory::TickMuse.
 * A note on takes its length from the next note off of its note. A piece cut
 * from the middle of the messages starts with the muses that restore the controller state in force.
 */
class MuseFactory {
public:
    /** One collected channel message. */
    struct TickMuse {
        int mTick;             /*!< The tick. */
        unsigned char mStatus; /*!< The status byte. */
        unsigned char mData1;  /*!< The first data byte. */
        unsigned char mData2;  /*!< The second data byte. */
        int mDuration;         /*!< The length of a note on in ticks, or -1 while unpaired. */
    };

    /**
     * Construct an empty collector.
     *
     * @ghidraAddress NTSC-U/C: 0x0015a9f8
     * @ghidraAddress PAL: 0x0015c1e8
     */
    MuseFactory();

    /**
     * Release the collector.
     *
     * @ghidraAddress NTSC-U/C: 0x0015aa68
     * @ghidraAddress PAL: 0x0015c258
     */
    ~MuseFactory();

    /**
     * Collect a channel message.
     *
     * The first message sets the channel.
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
     * An unpaired note on does not count.
     *
     * @param nStart The first tick of the span.
     * @param nEnd The tick after the span.
     * @return The number of messages.
     * @ghidraAddress NTSC-U/C: 0x0015ac28
     * @ghidraAddress PAL: 0x0015c418
     */
    int CountMessages(int nStart, int nEnd);

    /**
     * Build a piece from every collected message.
     *
     * @return The piece, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015acf8
     * @ghidraAddress PAL: 0x0015c4e8
     */
    MultiMuse *ExtractAll();

    /**
     * Build a piece from the messages in a span of ticks, starting at the first note on of the
     * span.
     *
     * @param nStart The first tick of the span.
     * @param nEnd The tick after the span.
     * @return The piece, with no reference taken.
     * @ghidraAddress NTSC-U/C: 0x0015ad98
     * @ghidraAddress PAL: 0x0015c588
     */
    MultiMuse *Extract(int nStart, int nEnd);

private:
    /**
     * Collect a note on. It waits for its note off.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The note.
     * @param nData2 The velocity.
     * @ghidraAddress NTSC-U/C: 0x0015b038
     * @ghidraAddress PAL: 0x0015c828
     */
    void AddNoteOn(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Pair a note off with the note on of its note.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The note.
     * @param nData2 The velocity.
     * @ghidraAddress NTSC-U/C: 0x0015b3b8
     * @ghidraAddress PAL: 0x0015cba8
     */
    void AddNoteOff(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Collect a message that is not a note.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0015b470
     * @ghidraAddress PAL: 0x0015cc60
     */
    void AddOther(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Build the muse of a message.
     *
     * @param message The message.
     * @return The muse, with no reference taken, or null for an unpaired note on.
     * @ghidraAddress NTSC-U/C: 0x0015b6a8
     * @ghidraAddress PAL: 0x0015ce98
     */
    Muse *MakeMuse(const TickMuse &message);

    std::vector<TickMuse> mMessages; /*!< The messages in the order they arrived. */
    std::vector<int> mOpenNotes;     /*!< The indices of the unpaired note ons. */
    int mChannel;                    /*!< The MIDI channel, or -1 before the first message. */
    MidiChannelState *mState;        /*!< The controller state, built with the first message. */
    int mResumeIndex;                /*!< The message the last Extract() started at, or -1. */
};
