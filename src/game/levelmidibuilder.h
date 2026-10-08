#pragma once

#include <vector>

#include "game/axetrackdata.h"
#include "game/backmusic.h"
#include "game/bankloader.h"
#include "game/catchtrackdata.h"
#include "game/fxmgr.h"
#include "game/lyric.h"
#include "game/pitchtrackriffdata.h"
#include "game/playmap.h"
#include "game/scratchtrackdata.h"
#include "game/scripttrackdata.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "game/worldtrack.h"
#include "gs/muse.h"
#include "mid/midireader.h"
#include "mid/midireceiver.h"
#include "mid/trackbuilder.h"
#include "os/file.h"
#include "os/ptr.h"
#include "os/string.h"

/**
 * Reader of the MIDI file of a song, which builds the data of every track it names.
 *
 * The class is not polymorphic. The name comes from the RTTI of its nested classes
 * BuilderReceiver and Tracks. The object is 0x10c bytes. Each MIDI track names its kind in its
 * name event, and the builder hands its events to a TrackBuilder for the kind. Track 0 is the
 * conductor track. A game track name looks like `T1 CATCH:D:Drums`, with the number of the
 * instrument track, its type, the instrument letter, and a label.
 */
class LevelMidiBuilder {
public:
    /** Kinds of a MIDI track, as its name identifies them. */
    enum TrackKind {
        kTrackKindNone = 0,       /*!< The name was not recognised. */
        kTrackKindConductor = 1,  /*!< Track 0, with the tempo and the time signature. */
        kTrackKindAxeContour = 2, /*!< `AXE_CONTOUR`, the notes of a guitar track. */
        kTrackKindAxeHarmony = 3, /*!< `AXE_HARMONY`, the harmony of the guitar track before. */
        kTrackKindPitch = 4,      /*!< `PITCH`, the riffs of a pitch track. */
        kTrackKindVox = 5,        /*!< `VOX`, the gems of a vocal track. */
        kTrackKindScratch = 6,    /*!< `SCRATCH1` to `SCRATCH3`, the patterns of a turntable. */
        kTrackKindCatch = 7,      /*!< `CATCH`, the gems of a catch track. */
        kTrackKindBackground = 8, /*!< A name starting `BG`, background music. */
        kTrackKindIntro = 9,      /*!< A name starting `INTRO`, music before the song. */
        kTrackKindWorld = 10,     /*!< `WORLD`, the events of the world. */
        kTrackKindScript = 11,    /*!< A name starting `SCRIPT`, the commands of a tutorial. */
        kTrackKindBank = 12,      /*!< A name starting `BANK`, the sound banks of the song. */
        kTrackKindIgnore = 13,    /*!< A name starting `IGNORE`, not read. */
    };

    /**
     * The data of one instrument track. Exactly one data pointer is set.
     *
     * The name comes from the RTTI. The object is 0x1c bytes.
     */
    class Tracks {
    public:
        /**
         * Hold the gems of a catch or vocal track.
         *
         * @param nType Song::kTrackTypeCatch or Song::kTrackTypeVox. Another type sets no data.
         * @param pGems The gems.
         * @param nInstrument The instrument of the track.
         * @ghidraAddress NTSC-U/C: 0x0011f1a8
         * @ghidraAddress PAL: 0x00120938
         */
        Tracks(int nType, CatchTrackData *pGems, int nInstrument);

        /**
         * Hold the patterns of a turntable track.
         *
         * @param pScratch The patterns.
         * @param nInstrument The instrument of the track.
         * @ghidraAddress NTSC-U/C: 0x0011f218
         * @ghidraAddress PAL: 0x001209a8
         */
        Tracks(ScratchTrackData *pScratch, int nInstrument);

        /**
         * Hold the notes of a guitar track.
         *
         * @param pAxe The notes.
         * @param nInstrument The instrument of the track.
         * @ghidraAddress NTSC-U/C: 0x0011f240
         * @ghidraAddress PAL: 0x001209d0
         */
        Tracks(AxeTrackData *pAxe, int nInstrument);

        /**
         * Hold the riffs of a pitch track, and set mFlags to 1.
         *
         * @param pRiffs The riffs.
         * @param nInstrument The instrument of the track.
         * @ghidraAddress NTSC-U/C: 0x0011f268
         * @ghidraAddress PAL: 0x001209f8
         */
        Tracks(PitchTrackRiffData *pRiffs, int nInstrument);

        /**
         * Delete the data the track holds.
         *
         * The name is inferred. The pointers are not cleared.
         *
         * @ghidraAddress NTSC-U/C: 0x0011f290
         * @ghidraAddress PAL: 0x00120a20
         */
        void Free();

        CatchTrackData *mCatch;     /*!< The gems of a catch track, or null. */
        ScratchTrackData *mScratch; /*!< The patterns of a turntable track, or null. */
        AxeTrackData *mAxe;         /*!< The notes of a guitar track, or null. */
        PitchTrackRiffData *mRiffs; /*!< The riffs of a pitch track, or null. */
        CatchTrackData *mVox;       /*!< The gems of a vocal track, or null. */
        int mFlags;                 /*!< 1 for a pitch track, otherwise 0. */
        int mInstrument;            /*!< The instrument of the track. */
    };

    /** MidiReceiver that passes the events of the MIDI file to the builder. */
    class BuilderReceiver : public MidiReceiver {
    public:
        /**
         * Construct a receiver for a builder.
         *
         * @param pOwner The builder.
         */
        explicit BuilderReceiver(LevelMidiBuilder *pOwner) : mOwner(pOwner) {
        }

        /** Release the receiver. */
        ~BuilderReceiver() override {
        }

        /**
         * Pass the start of a track to the builder.
         *
         * @param nTrack The track.
         */
        void OnNewTrack(unsigned char nTrack) override {
            mOwner->OnNewTrack(nTrack);
        }

        /** Pass the end of a track to the builder. */
        void OnEndTrack() override {
            mOwner->OnEndTrack();
        }

        /** Pass the end of the file to the builder. */
        void OnAllDone() override {
            mOwner->OnAllDone();
        }

        /**
         * Pass a channel message to the builder.
         *
         * @param nTick The tick of the message.
         * @param nStatus The status byte.
         * @param nData1 The first data byte.
         * @param nData2 The second data byte.
         */
        void OnMidi(int nTick,
                    unsigned char nStatus,
                    unsigned char nData1,
                    unsigned char nData2) override {
            mOwner->OnMidi(nTick, nStatus, nData1, nData2);
        }

        /**
         * Pass a tempo change to the builder.
         *
         * @param nTick The tick.
         * @param nMicrosecondsPerBeat The tempo.
         */
        void OnTempo(int nTick, int nMicrosecondsPerBeat) override {
            mOwner->OnTempo(nTick, nMicrosecondsPerBeat);
        }

        /**
         * Pass a text event to the builder.
         *
         * @param nTick The tick.
         * @param pszText The text.
         * @param nType The type of the event.
         */
        void OnText(int nTick, const char *pszText, unsigned char nType) override {
            mOwner->OnText(nTick, pszText, nType);
        }

        /**
         * Pass a time signature to the builder.
         *
         * @param nTick The tick.
         * @param nNumerator The beats in a bar.
         * @param nDenominator The length of a beat.
         */
        void OnTimeSignature(int nTick, int nNumerator, int nDenominator) override {
            mOwner->OnTimeSignature(nTick, nNumerator, nDenominator);
        }

    private:
        LevelMidiBuilder *mOwner; /*!< The builder. */
    };

    /**
     * Construct a builder for the MIDI file of a song.
     *
     * @param pszMidiFile The MIDI file, which the caller retains.
     * @param nDifficulty The difficulty, which the catch tracks build their gems for.
     * @param nRuleSet The rule set of the game, one of GameDb::RuleSet.
     * @param nNumBars The length of the song in bars.
     * @param nIntroBars The bars before bar 0.
     * @param nFreestyleEffectsBus The effects bus of the guitar tracks.
     * @param nRemixEffectsBus The effects bus of a remix.
     * @param pSections The sections of the song.
     * @param pMsPerTick The duration of one tick in milliseconds.
     * @param pSlotGrid The patterns of the sections.
     * @ghidraAddress NTSC-U/C: 0x0011f318
     * @ghidraAddress PAL: 0x00120aa8
     */
    LevelMidiBuilder(const char *pszMidiFile,
                     int nDifficulty,
                     int nRuleSet,
                     int nNumBars,
                     int nIntroBars,
                     int nFreestyleEffectsBus,
                     int nRemixEffectsBus,
                     SectionBoundaries *pSections,
                     float *pMsPerTick,
                     SlotGrid *pSlotGrid);

    /**
     * Release the builder, the file, and the data of every track.
     *
     * @ghidraAddress NTSC-U/C: 0x0011f4e0
     * @ghidraAddress PAL: 0x00120c70
     */
    ~LevelMidiBuilder();

    /**
     * Start reading the MIDI file without checking it for authoring errors.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0011f888
     * @ghidraAddress PAL: 0x00121018
     */
    void StartLoad();

    /**
     * Start reading the MIDI file and check it for authoring errors.
     *
     * The name is inferred.
     *
     * @param pfnError The routine that reports an error.
     * @ghidraAddress NTSC-U/C: 0x0011f8b0
     * @ghidraAddress PAL: 0x00121040
     */
    void StartLoad(TrackBuilder::ErrorHandler pfnError);

    /**
     * Advance the read of the MIDI file, parsing it for up to 15 milliseconds once it is in
     * memory, and release the file once it is parsed.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0011f9a8
     * @ghidraAddress PAL: 0x00121138
     */
    void Poll();

    /**
     * Report whether the MIDI file is parsed.
     *
     * The name is inferred.
     *
     * @return mDone.
     * @ghidraAddress NTSC-U/C: 0x0011faa0
     * @ghidraAddress PAL: 0x00121230
     */
    int IsDone() const;

    /**
     * Start a track. Track 0 is the conductor track. Another track waits for its name.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x0011faa8
     * @ghidraAddress PAL: 0x00121238
     */
    void OnNewTrack(unsigned char nTrack);

    /**
     * Finish the track, and make the lyrics after the conductor track.
     *
     * @ghidraAddress NTSC-U/C: 0x0011fb00
     * @ghidraAddress PAL: 0x00121290
     */
    void OnEndTrack();

    /**
     * Report a song without a WORLD track or a BANK track.
     *
     * @ghidraAddress NTSC-U/C: 0x0011fb98
     * @ghidraAddress PAL: 0x00121328
     */
    void OnAllDone();

    /**
     * Pass a channel message to the builder of the track.
     *
     * @param nTick The tick of the message.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0011fc28
     * @ghidraAddress PAL: 0x001213b8
     */
    void OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Pass a tempo change to the builder of the track.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The tempo.
     * @ghidraAddress NTSC-U/C: 0x0011fc68
     * @ghidraAddress PAL: 0x001213f8
     */
    void OnTempo(int nTick, int nMicrosecondsPerBeat);

    /**
     * Pass a time signature to the builder of the track.
     *
     * @param nTick The tick.
     * @param nNumerator The beats in a bar.
     * @param nDenominator The length of a beat.
     * @ghidraAddress NTSC-U/C: 0x0011fca0
     * @ghidraAddress PAL: 0x00121430
     */
    void OnTimeSignature(int nTick, int nNumerator, int nDenominator);

    /**
     * Take the name of a track that has none yet, and build its builder. Pass every other text
     * event to the builder of the track.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The type of the event, 3 for a track name.
     * @ghidraAddress NTSC-U/C: 0x0011fcd8
     * @ghidraAddress PAL: 0x00121468
     */
    void OnText(int nTick, const char *pszText, unsigned char nType);

    /**
     * Replace the data of the tracks with the saved remix of the game database.
     *
     * The name is inferred. The remix holds the speed, the effects, and the gems of each pitch
     * and vocal track. A pitch track becomes a catch track of the riffs its gems pick.
     *
     * @ghidraAddress NTSC-U/C: 0x00122088
     * @ghidraAddress PAL: 0x00123818
     */
    void LoadRemix();

    /**
     * Report the number of instrument tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x001223e0
     * @ghidraAddress PAL: 0x00123b60
     */
    int GetNumTracks() const;

    /**
     * Report the type of an instrument track.
     *
     * @param nTrack The track.
     * @return One of Song::TrackType.
     * @ghidraAddress NTSC-U/C: 0x00122400
     * @ghidraAddress PAL: 0x00123b80
     */
    int GetTrackType(int nTrack) const;

    /**
     * Report the instrument of a track.
     *
     * @param nTrack The track.
     * @return The instrument.
     * @ghidraAddress NTSC-U/C: 0x00122458
     * @ghidraAddress PAL: 0x00123bd8
     */
    int GetTrackInstrument(int nTrack) const;

    /**
     * Report the gems of a catch track.
     *
     * @param nTrack The track.
     * @return The gems, or null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x00122470
     * @ghidraAddress PAL: 0x00123bf0
     */
    CatchTrackData *GetCatchTrackData(int nTrack) const;

    /**
     * Report the patterns of a turntable track.
     *
     * @param nTrack The track.
     * @return The patterns, or null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x00122488
     * @ghidraAddress PAL: 0x00123c08
     */
    ScratchTrackData *GetScratchData(int nTrack) const;

    /**
     * Report the notes of a guitar track.
     *
     * @param nTrack The track.
     * @return The notes, or null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x001224a0
     * @ghidraAddress PAL: 0x00123c20
     */
    AxeTrackData *GetAxeData(int nTrack) const;

    /**
     * Report the riffs of a pitch track.
     *
     * @param nTrack The track.
     * @return The riffs, or null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x001224b8
     * @ghidraAddress PAL: 0x00123c38
     */
    PitchTrackRiffData *GetTrackRiffData(int nTrack) const;

    /**
     * Report the gems of a vocal track.
     *
     * @param nTrack The track.
     * @return The gems, or null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x001224d0
     * @ghidraAddress PAL: 0x00123c50
     */
    CatchTrackData *GetVoxData(int nTrack) const;

    /**
     * Report the flags of a track.
     *
     * @param nTrack The track.
     * @return The flags.
     * @ghidraAddress NTSC-U/C: 0x001224e8
     * @ghidraAddress PAL: 0x00123c68
     */
    int GetTrackFlags(int nTrack) const;

    /**
     * Report the number of background music tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x00122500
     * @ghidraAddress PAL: 0x00123c80
     */
    int GetNumBackMusic() const;

    /**
     * Report one background music track.
     *
     * @param nIndex The index.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x00122518
     * @ghidraAddress PAL: 0x00123c98
     */
    BackMusic *GetBackMusic(int nIndex) const;

    /**
     * Report the number of performances of the tracks named "INTRO".
     *
     * @return The number of performances.
     * @ghidraAddress NTSC-U/C: 0x00122530
     * @ghidraAddress PAL: 0x00123cb0
     */
    int GetNumIntroMuses() const;

    /**
     * Report the performance of one track named "INTRO".
     *
     * @param nIndex The index.
     * @return The performance.
     * @ghidraAddress NTSC-U/C: 0x00122548
     * @ghidraAddress PAL: 0x00123cc8
     */
    Muse *GetIntroMuse(int nIndex) const;

    /**
     * Find the commands of a track named "SCRIPT" by their name.
     *
     * The name is inferred.
     *
     * @param pszName The name.
     * @return The commands, or null.
     * @ghidraAddress NTSC-U/C: 0x00122570
     * @ghidraAddress PAL: 0x00123cf0
     */
    ScriptTrackData *FindScriptTrack(const char *pszName) const;

    /**
     * Format an error message with the MIDI track and the position it concerns.
     *
     * @param prefix The text the message starts with.
     * @param nTick The tick of the error.
     * @param nTrack The MIDI track.
     * @param message The error.
     * @return The message.
     * @ghidraAddress NTSC-U/C: 0x00121e80
     * @ghidraAddress PAL: 0x00123610
     */
    static String FormatError(const String &prefix, int nTick, int nTrack, const String &message);

    std::vector<Tracks> mTracks;                  /*!< The instrument tracks. */
    std::vector<BackMusic *> mBackMusic;          /*!< The background music tracks. */
    std::vector<Ptr<Muse>> mIntroMuses;           /*!< The performances of the INTRO tracks. */
    std::vector<ScriptTrackData *> mScriptTracks; /*!< The commands of the SCRIPT tracks. */
    PlayMap *mPlayMap;                            /*!< The map of the song positions. */
    BankLoader *mBankTrack;                       /*!< The track named "BANK", or null. */
    WorldTrack mWorldTrack;                       /*!< The events of the track named "WORLD". */
    Lyric *mLyric;                                /*!< The lyrics. */
    FXMgr *mFXMgr;                                /*!< The effect sets of a remix, or null. */
    float mSpeed;                                 /*!< The speed of the song. */
    char *mBuffer;                                /*!< The contents of the MIDI file. */
    int mSize;                                    /*!< The size of the MIDI file in bytes. */
    File *mFile;                                  /*!< The MIDI file while it is read. */
    MidiReader *mReader;                          /*!< The parser while it parses. */
    String mDirectory;                            /*!< The directory of the MIDI file. */
    int mReadDone;                                /*!< Whether the MIDI file is in memory. */
    int mDone;                                    /*!< Whether the MIDI file is parsed. */
    const char *mMidiFile;                        /*!< The MIDI file. */
    int mDifficulty;                              /*!< The difficulty. */
    int mTicksPerBar;                             /*!< The length of a bar in ticks. */
    int mNumBars;                                 /*!< The length of the song in bars. */
    int mIntroBars;                               /*!< The bars before bar 0. */
    int mFreestyleEffectsBus;                     /*!< The effects bus of the guitar tracks. */
    int mRemixEffectsBus;                         /*!< The effects bus of a remix. */
    BuilderReceiver *mReceiver;                   /*!< The receiver of the parser. */
    int mValidate;                                /*!< Whether errors are reported. */
    TrackBuilder::ErrorHandler mErrorHandler;     /*!< The routine that reports an error. */
    int mTrackIndex;                              /*!< The MIDI track being parsed. */
    int mGameTrackCount;                          /*!< The instrument tracks parsed so far. */
    String mTrackName;                            /*!< The name of the MIDI track. */
    String mTrackLabel;                           /*!< The label after the instrument letter. */
    int mInstrument;                              /*!< The instrument of the MIDI track. */
    int mTrackKind;                               /*!< The kind of the MIDI track, a TrackKind. */
    int mScratchIndex;                            /*!< The last turntable track, or -1. */
    int mHasWorldTrack;                           /*!< Whether a WORLD track was found. */
    TrackBuilder *mTrackBuilder;                  /*!< The builder of the MIDI track, or null. */
    int mRuleSet;                                 /*!< The rule set, one of GameDb::RuleSet. */
    SectionBoundaries *mSections;                 /*!< The sections of the song. */
    float *mMsPerTick;                            /*!< The duration of one tick in milliseconds. */
    SlotGrid *mSlotGrid;                          /*!< The patterns of the sections. */

private:
    /**
     * Open the MIDI file and start reading it.
     *
     * @ghidraAddress NTSC-U/C: 0x0011f8d8
     * @ghidraAddress PAL: 0x00121068
     */
    void OpenFile();

    /**
     * Find the kind of a MIDI track from its name.
     *
     * @param pszName The name.
     * @ghidraAddress NTSC-U/C: 0x0011fda8
     * @ghidraAddress PAL: 0x00121538
     */
    void ClassifyTrack(const char *pszName);

    /**
     * Read the number, the type, and the instrument from the name of a game track.
     *
     * @param name The name, in capitals.
     * @ghidraAddress NTSC-U/C: 0x00120018
     * @ghidraAddress PAL: 0x001217a8
     */
    void ParseGameTrackName(const String &name);

    /**
     * Make the data and the builder of the MIDI track for its kind.
     *
     * @ghidraAddress NTSC-U/C: 0x00120968
     * @ghidraAddress PAL: 0x001220f8
     */
    void CreateTrackBuilder();

    /**
     * Report an error in the MIDI file.
     *
     * @param nTick The tick of the error.
     * @param message The error.
     * @ghidraAddress NTSC-U/C: 0x00121fc8
     * @ghidraAddress PAL: 0x00123758
     */
    void Error(int nTick, const String &message);
};
