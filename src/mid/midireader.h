#pragma once

#include <vector>

#include "mid/midireceiver.h"
#include "os/binstream.h"

/**
 * Reader of a Standard MIDI File that reports each track and event to a MidiReceiver.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The reader advances one step
 * at a time: the header, the start of a track, or one event. Event positions are rescaled from
 * the division of the file to 480 ticks per quarter note. The channel events of one position are
 * collected and sorted with mCompare before they are reported.
 */
class MidiReader {
public:
    /** One channel event waiting in mPending. */
    struct Midi {
        unsigned char mStatus; /*!< The status byte. */
        unsigned char mData1;  /*!< The first data byte. */
        unsigned char mData2;  /*!< The second data byte, or 0 for a one-byte event. */
    };

    /** Ordering of mPending. The result is true when the first event sorts first. */
    using Compare = bool (*)(const Midi &left, const Midi &right);

    /** The four-character name of a chunk. */
    struct ChunkName {
        /** The characters of a name. */
        static constexpr int kLength = 4;

        /**
         * Read the name.
         *
         * @param stream The stream to read from.
         * @return The stream.
         * @ghidraAddress NTSC-U/C: 0x0034e140
         */
        BinStream &Read(BinStream &stream);

        char mName[kLength]; /*!< The characters, with no terminator. */
    };

    /** A variable-length quantity, read seven bits a byte from the most significant. */
    struct VarLen {
        /**
         * Read a quantity.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x00159f68
         * @ghidraAddress PAL: 0x0015b758
         */
        explicit VarLen(BinStream &stream);

        /**
         * Read the quantity.
         *
         * @param stream The stream to read from.
         * @return The stream.
         * @ghidraAddress NTSC-U/C: 0x00159f90
         * @ghidraAddress PAL: 0x0015b788
         */
        BinStream &Read(BinStream &stream);

        int mValue; /*!< The quantity. */
    };

    /** Values of mState. */
    enum State {
        kStateHeader = 0,     /*!< The header is next. */
        kStateTrackStart = 1, /*!< The start of a track is next. */
        kStateEvent = 2,      /*!< An event of the current track is next. */
        kStateDone = 3,       /*!< The file is read. */
    };

    /**
     * Order channel events so note offs come first, then controllers, programs, channel pressure,
     * pitch bends, key pressure, and note ons, and any other message last.
     *
     * @param left The first event.
     * @param right The second event.
     * @return Whether the first event sorts first.
     * @ghidraAddress NTSC-U/C: 0x00158e50
     * @ghidraAddress PAL: 0x0015a690
     */
    static bool CompareMidi(const Midi &left, const Midi &right);

    /**
     * Open a MIDI file.
     *
     * @param pszFile The file.
     * @param pReceiver The receiver of the tracks and events.
     * @ghidraAddress NTSC-U/C: 0x00158fa8
     * @ghidraAddress PAL: 0x0015a830
     */
    MidiReader(const char *pszFile, MidiReceiver *pReceiver);

    /**
     * Read a MIDI file from memory.
     *
     * @param pBuffer The contents of the file. The caller retains them.
     * @param nSize The size of the contents in bytes.
     * @param pReceiver The receiver of the tracks and events.
     * @ghidraAddress NTSC-U/C: 0x00159048
     * @ghidraAddress PAL: 0x0015a8d0
     */
    MidiReader(const char *pBuffer, int nSize, MidiReceiver *pReceiver);

    /**
     * Close the stream.
     *
     * @ghidraAddress NTSC-U/C: 0x001590f0
     * @ghidraAddress PAL: 0x0015a978
     */
    ~MidiReader();

    /**
     * Read the whole file.
     *
     * @ghidraAddress NTSC-U/C: 0x00159198
     * @ghidraAddress PAL: 0x0015aa20
     */
    void ReadAll();

    /**
     * Read up to the end of the next track.
     *
     * @return Whether a track ended, and false at the end of the file.
     * @ghidraAddress NTSC-U/C: 0x001591d0
     * @ghidraAddress PAL: 0x0015aa58
     */
    bool ReadTrack();

    /**
     * Take a number of steps.
     *
     * @param nSteps The number of steps.
     * @return Whether the file is not yet read.
     * @ghidraAddress NTSC-U/C: 0x00159228
     * @ghidraAddress PAL: 0x0015aab0
     */
    bool ReadSteps(int nSteps);

    /**
     * Take steps until a time has passed.
     *
     * @param fMs The time in milliseconds.
     * @return Whether the file is not yet read.
     * @ghidraAddress NTSC-U/C: 0x001592a0
     * @ghidraAddress PAL: 0x0015ab28
     */
    bool ReadFor(float fMs);

private:
    /**
     * Read the header, the start of a track, or one event, as mState selects.
     *
     * @ghidraAddress NTSC-U/C: 0x001593b8
     * @ghidraAddress PAL: 0x0015ac40
     */
    void Step();

    /**
     * Read the `MThd` chunk.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00159428
     * @ghidraAddress PAL: 0x0015acb0
     */
    void ReadHeader(BinStream &stream);

    /**
     * Read the start of an `MTrk` chunk and report the track.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x001594e0
     * @ghidraAddress PAL: 0x0015ad68
     */
    void BeginTrack(BinStream &stream);

    /**
     * Read one delta time and event.
     *
     * A data byte in the status position repeats the running status.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00159580
     * @ghidraAddress PAL: 0x0015ae08
     */
    void ReadEvent(BinStream &stream);

    /**
     * Read the rest of a channel event and collect it.
     *
     * A note on with a velocity of 0 becomes a note off.
     *
     * @param nTick The position.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x001596b8
     */
    void
    ReadChannelEvent(int nTick, unsigned char nStatus, unsigned char nData1, BinStream &stream);

    /**
     * Read a system-exclusive or meta event.
     *
     * @param nTick The position.
     * @param nStatus The status byte.
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00159800
     * @ghidraAddress PAL: 0x0015b058
     */
    void ReadSystemEvent(int nTick, unsigned char nStatus, BinStream &stream);

    /**
     * Read one meta event and report the end of a track, a tempo, a time signature, or a text.
     *
     * The stream is left after the data of the event whatever the type.
     *
     * @param nTick The position.
     * @param nType The meta type.
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x001598c8
     * @ghidraAddress PAL: 0x0015b118
     */
    void ReadMeta(int nTick, unsigned char nType, BinStream &stream);

    /**
     * Report a channel event, or collect it in mPending when mCompare is set.
     *
     * @param nTick The position.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00159bb0
     * @ghidraAddress PAL: 0x0015b3a0
     */
    void AddMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Sort the collected events and report them.
     *
     * @ghidraAddress NTSC-U/C: 0x00159e10
     * @ghidraAddress PAL: 0x0015b600
     */
    void Flush();

    BinStream *mStream;           /*!< The stream of the file. */
    MidiReceiver *mReceiver;      /*!< The receiver of the tracks and events. */
    int mState;                   /*!< One of State. */
    short mNumTracks;             /*!< The tracks of the file. */
    short mDivision;              /*!< The ticks per quarter note of the file. */
    short mTargetDivision;        /*!< The ticks per quarter note of the reported positions. */
    int mTrackIndex;              /*!< The tracks started so far. */
    int mTrackTick;               /*!< The position in the current track, in file ticks. */
    unsigned char mRunningStatus; /*!< The running status. */
    std::vector<Midi> mPending;   /*!< The channel events of the current position. */
    int mPendingTick;             /*!< The position of mPending, or -1. */
    Compare mCompare;             /*!< The ordering of mPending, or null to report at once. */
};
