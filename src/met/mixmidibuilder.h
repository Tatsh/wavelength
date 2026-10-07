#pragma once

#include <set>
#include <vector>

#include "gs/muse.h"
#include "gs/musebuilder.h"
#include "mid/midireader.h"
#include "mid/midireceiver.h"
#include "os/file.h"
#include "os/ptr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Reader of the MIDI file of a menu song that builds a piece of music for each track a mix of the
 * song lists.
 *
 * The RTTI includes the class name in the nested classes MixMidiBuilder::BuilderReceiver and
 * MixMidiBuilder::TrackData. The class is not polymorphic. The object is 0x7c bytes. The file is
 * read a part at a time.
 */
class MixMidiBuilder {
public:
    /**
     * Receiver that hands every event of the file to its builder.
     *
     * The RTTI includes the nested name and records MidiReceiver as the base. Every member is
     * inline.
     */
    class BuilderReceiver : public MidiReceiver {
    public:
        /**
         * Construct the receiver of a builder.
         *
         * @param pOwner The builder.
         */
        explicit BuilderReceiver(MixMidiBuilder *pOwner) : mOwner(pOwner) {
        }

        /**
         * Release the receiver.
         *
         * @ghidraAddress NTSC-U/C: 0x00356448
         * @ghidraAddress PAL: 0x003c36f8
         */
        ~BuilderReceiver() override {
        }

        /**
         * Begin a track.
         *
         * @param nTrack The track index.
         * @ghidraAddress NTSC-U/C: 0x003564c8
         * @ghidraAddress PAL: 0x003c3778
         */
        void OnNewTrack(unsigned char nTrack) override {
            mOwner->OnNewTrack(nTrack);
        }

        /**
         * End the track.
         *
         * @ghidraAddress NTSC-U/C: 0x003564e8
         * @ghidraAddress PAL: 0x003c3798
         */
        void OnEndTrack() override {
            mOwner->OnEndTrack();
        }

        /**
         * End the file.
         *
         * @ghidraAddress NTSC-U/C: 0x00356508
         * @ghidraAddress PAL: 0x003c37b8
         */
        void OnAllDone() override {
            mOwner->OnAllDone();
        }

        /**
         * Report a channel message.
         *
         * @param nTick The tick.
         * @param nStatus The status byte.
         * @param nData1 The first data byte.
         * @param nData2 The second data byte.
         * @ghidraAddress NTSC-U/C: 0x00356528
         * @ghidraAddress PAL: 0x003c37d8
         */
        void OnMidi(int nTick,
                    unsigned char nStatus,
                    unsigned char nData1,
                    unsigned char nData2) override {
            mOwner->OnMidi(nTick, nStatus, nData1, nData2);
        }

        /**
         * Report a tempo change.
         *
         * @param nTick The tick.
         * @param nMicrosecondsPerBeat The new tempo.
         * @ghidraAddress NTSC-U/C: 0x00356550
         * @ghidraAddress PAL: 0x003c3800
         */
        void OnTempo(int nTick, int nMicrosecondsPerBeat) override {
            mOwner->OnTempo(nTick, nMicrosecondsPerBeat);
        }

        /**
         * Report a text meta event.
         *
         * @param nTick The tick.
         * @param pszText The text.
         * @param nType The meta event type.
         * @ghidraAddress NTSC-U/C: 0x00356570
         * @ghidraAddress PAL: 0x003c3820
         */
        void OnText(int nTick, const char *pszText, unsigned char nType) override {
            mOwner->OnText(nTick, pszText, nType);
        }

        /**
         * Report a time signature.
         *
         * @param nTick The tick.
         * @param nNumerator The beats per bar.
         * @param nDenominator The beat unit.
         * @ghidraAddress NTSC-U/C: 0x00356590
         * @ghidraAddress PAL: 0x003c3840
         */
        void OnTimeSignature(int nTick, int nNumerator, int nDenominator) override {
            mOwner->OnTimeSignature(nTick, nNumerator, nDenominator);
        }

    private:
        MixMidiBuilder *mOwner; /*!< The builder. */
    };

    /** One piece of music built from a track of the file. */
    struct TrackData {
        Ptr<Muse> mMuse;        /*!< The piece. */
        unsigned char mChannel; /*!< The MIDI channel of the track. */
        String mName;           /*!< The name of the track. */
    };

    /**
     * Collect the track names of the `mix` entry of a menu song.
     *
     * @param pConfig The entry of the song.
     * @ghidraAddress NTSC-U/C: 0x0016aab0
     * @ghidraAddress PAL: 0x0016dc38
     */
    explicit MixMidiBuilder(DataArray *pConfig);

    /**
     * Release the pieces and the reader.
     *
     * @ghidraAddress NTSC-U/C: 0x0016acf0
     * @ghidraAddress PAL: 0x0016de78
     */
    ~MixMidiBuilder();

    /**
     * Start reading the file the `music_shared_midi_file` entry of the metagame configuration
     * specifies into a buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x0016ae78
     * @ghidraAddress PAL: 0x0016e000
     */
    void StartLoad();

    /**
     * Read more of the file, or parse more of its events once it has been read.
     *
     * @ghidraAddress NTSC-U/C: 0x0016af58
     * @ghidraAddress PAL: 0x0016e0e0
     */
    void Poll();

    /**
     * Report whether every event has been parsed.
     *
     * @return Whether the pieces are built.
     * @ghidraAddress NTSC-U/C: 0x0016b048
     * @ghidraAddress PAL: 0x0016e1d0
     */
    bool IsDone() const;

    /**
     * Report the number of pieces built.
     *
     * @return The number of pieces.
     * @ghidraAddress NTSC-U/C: 0x0016b050
     * @ghidraAddress PAL: 0x0016e1d8
     */
    int NumTracks() const;

    /**
     * Report a piece.
     *
     * @param nTrack The index of the piece.
     * @return The piece.
     * @ghidraAddress NTSC-U/C: 0x0016b070
     * @ghidraAddress PAL: 0x0016e1f8
     */
    Muse *TrackMuse(int nTrack) const;

    /**
     * Report the MIDI channel of a piece.
     *
     * @param nTrack The index of the piece.
     * @return The channel.
     * @ghidraAddress NTSC-U/C: 0x0016b098
     * @ghidraAddress PAL: 0x0016e220
     */
    unsigned char TrackChannel(int nTrack) const;

    /**
     * Report the name of the track of a piece.
     *
     * @param nTrack The index of the piece.
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0016b0b0
     * @ghidraAddress PAL: 0x0016e238
     */
    const String &TrackName(int nTrack) const;

private:
    /**
     * Forget the name of the previous track.
     *
     * @param nTrack The track index. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x0016b0c8
     * @ghidraAddress PAL: 0x0016e250
     */
    void OnNewTrack(unsigned char nTrack);

    /**
     * Finish the piece of the track, name it after the track, and count the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0016b0f0
     * @ghidraAddress PAL: 0x0016e278
     */
    void OnEndTrack();

    /**
     * Do nothing at the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x0016b178
     * @ghidraAddress PAL: 0x0016e300
     */
    void OnAllDone();

    /**
     * Record the channel of the track a mix lists, and pass the message to its builder.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0016b180
     * @ghidraAddress PAL: 0x0016e308
     */
    void OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Record the length of a beat, and pass the tempo change to the builder of the track.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x0016b1d8
     * @ghidraAddress PAL: 0x0016e360
     */
    void OnTempo(int nTick, int nMicrosecondsPerBeat);

    /**
     * Start a piece when the name of the track is one a mix lists, and pass the event to the
     * builder of the track.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x0016b228
     * @ghidraAddress PAL: 0x0016e3b0
     */
    void OnText(int nTick, const char *pszText, unsigned char nType);

    /**
     * Record the length of a bar, and pass the time signature to the builder of the track.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x0016b5f8
     * @ghidraAddress PAL: 0x0016e780
     */
    void OnTimeSignature(int nTick, int nNumerator, int nDenominator);

    std::vector<TrackData> mTracks; /*!< The pieces, in the order of their tracks. */
    char *mBuffer;                  /*!< The contents of the file, or null. */
    int mSize;                      /*!< The size of the file in bytes. */
    File *mFile;                    /*!< The file while it is read, or null. */
    String mReserved1c;             // +0x1c, constructed and destroyed, not otherwise used here.
    int mReadDone;                  /*!< Whether the file has been read. */
    int mDone;                      /*!< Whether every event has been parsed. */
    MidiReader *mReader;            /*!< The parser of the events, or null. */
    std::set<String> mNames;        /*!< The names of the tracks the mixes list. */
    BuilderReceiver *mReceiver;     /*!< The receiver the parser reports to. */
    int mReserved50;                // +0x50, cleared by the constructor, not otherwise used here.
    int mTrackIndex;                /*!< The index of the current track, or -1. */
    String mTrackName;              /*!< The name of the current track. */
    float mMillisecondsPerBeat;     /*!< The length of a beat, or -1. */
    int mTicksPerBar;               /*!< The length of a bar in ticks, or -1. */
    MuseBuilder *mMuseBuilder;      /*!< The builder of the current piece, or null. */
    int mHasTrack;                  /*!< Whether the current track has a piece. */
};
