#pragma once

#include <vector>

#include "game/axetrackdata.h"
#include "game/backmusic.h"
#include "game/bankloader.h"
#include "game/catchtrackdata.h"
#include "game/duelpatterntable.h"
#include "game/fxmgr.h"
#include "game/levelmidibuilder.h"
#include "game/lyric.h"
#include "game/pitchtrackgems.h"
#include "game/pitchtrackriffdata.h"
#include "game/playmap.h"
#include "game/scratchtrackdata.h"
#include "game/scripttrackdata.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "game/worldtrack.h"
#include "gs/muse.h"
#include "mid/trackbuilder.h"
#include "os/file.h"
#include "os/mem.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "synth/syntheffects.h"

/**
 * One song: its configuration from the song text file and the tracks its MIDI file builds.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the song text file it
 * reads. The object is 0x370 bytes. Load() reads the configuration and starts the MIDI file, and
 * PollLoad() advances the load through the states of LoadState until IsLoaded() reports it done. A
 * remix or a duel also reads the gem patterns of its sections from three pattern files.
 */
class Song {
public:
    /** Types of an instrument track. */
    enum TrackType {
        kTrackTypeAxe = 1,     /*!< A guitar freestyle track. */
        kTrackTypePitch = 2,   /*!< A pitch track. */
        kTrackTypeScratch = 3, /*!< A turntable freestyle track. */
        kTrackTypeVox = 4,     /*!< A vocal track. */
        kTrackTypeCatch = 5,   /*!< A catch track. */
    };

    /** Values of mState. */
    enum LoadState {
        kLoadStateParse = 0, /*!< The MIDI file is parsed. */
        kLoadStateSetup = 1, /*!< The MIDI file is parsed, and the track settings are read next. */
        kLoadStatePatterns = 2, /*!< The pattern files are read. */
        kLoadStateDone = 3,     /*!< The song is loaded. */
    };

    /** Number of mixer channels the song can set the low volumes of. */
    static constexpr int kNumMixerChannels = 16;

    /** Number of pattern files a remix or a duel reads, and of gem patterns of a pitch track. */
    static constexpr int kNumPatternFiles = 3;

    /**
     * One pattern file of a remix or a duel while it is read.
     *
     * The name is inferred. The object is 0x2c bytes.
     */
    class PatternFile {
    public:
        /**
         * Construct the record of a file.
         *
         * @param pszPath The path of the file.
         */
        explicit PatternFile(const char *pszPath) : mPath(pszPath) {
        }

        /** Release the file and its contents. */
        ~PatternFile() {
            delete mFile;
            PoolMemFree(mBuffer);
        }

        String mPath;  /*!< The path of the file. */
        int mIndex;    /*!< The pattern the file holds, from 0. */
        Song *mSong;   /*!< The song. */
        File *mFile;   /*!< The file, or null when it was not found. */
        char *mBuffer; /*!< The contents of the file. */
        int mSize;     /*!< The size of the file in bytes. */
        int mDone;     /*!< Whether the file is read or was not found. */
    };

    /**
     * Read the configuration of a song for a difficulty and a rule set.
     *
     * @param pSongConfig The entry of the song in the "songs" section.
     * @param nSkillLevel The difficulty.
     * @param nRuleSet The rule set the song is played under, one of GameDb::RuleSet.
     * @ghidraAddress NTSC-U/C: 0x00118e00
     * @ghidraAddress PAL: 0x0011a598
     */
    Song(DataArray *pSongConfig, int nSkillLevel, int nRuleSet);

    /**
     * Release the song and its tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x00119008
     * @ghidraAddress PAL: 0x0011a7a0
     */
    ~Song();

    /**
     * Start loading the song without checking it for authoring errors.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001194b0
     * @ghidraAddress PAL: 0x0011ac48
     */
    void Load();

    /**
     * Advance the load Load() started.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00119568
     * @ghidraAddress PAL: 0x0011ad00
     */
    void PollLoad();

    /**
     * Report whether the load Load() started is complete.
     *
     * The name is inferred.
     *
     * @return Whether the song is loaded.
     * @ghidraAddress NTSC-U/C: 0x001197d8
     * @ghidraAddress PAL: 0x0011af68
     */
    bool IsLoaded();

    /**
     * Replace the tracks with the saved remix the game database selects.
     *
     * The name is inferred. The pitch tracks lose their pattern gems, every instrument track is
     * enabled in one step, and the song splits into four sections at the quarter points.
     *
     * @ghidraAddress NTSC-U/C: 0x0011e880
     * @ghidraAddress PAL: 0x00120010
     */
    void ApplySavedRemix();

    /**
     * Apply the effects buses of the song, and set controller 16 of every MIDI channel to the
     * level of the channel's instrument.
     *
     * The name is inferred. A guitar or turntable track gets 50, a channel without a track the
     * level of the channel before it, and channel 16 gets 70.
     *
     * @ghidraAddress NTSC-U/C: 0x0011e738
     * @ghidraAddress PAL: 0x0011fec8
     */
    void ApplySynthSettings();

    /**
     * Report the number of instrument tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0011ede0
     * @ghidraAddress PAL: 0x00120570
     */
    int GetNumTracks() const;

    /**
     * Report the type of an instrument track.
     *
     * @param nTrack The track.
     * @return One of TrackType.
     * @ghidraAddress NTSC-U/C: 0x0011ee00
     * @ghidraAddress PAL: 0x00120590
     */
    int GetTrackType(int nTrack) const;

    /**
     * Report the instrument of a track.
     *
     * @param nTrack The track.
     * @return The instrument.
     * @ghidraAddress NTSC-U/C: 0x0011ee20
     * @ghidraAddress PAL: 0x001205b0
     */
    int GetTrackInstrument(int nTrack) const;

    /**
     * Report the duel patterns of the song.
     *
     * The name is inferred.
     *
     * @return The patterns.
     * @ghidraAddress NTSC-U/C: 0x0011ee40
     * @ghidraAddress PAL: 0x001205d0
     */
    DuelPatternTable *GetDuelPatterns();

    /**
     * Report the gems of a catch track.
     *
     * @param nTrack The track.
     * @return The gems.
     * @ghidraAddress NTSC-U/C: 0x0011ee48
     * @ghidraAddress PAL: 0x001205d8
     */
    CatchTrackData *GetCatchTrackData(int nTrack) const;

    /**
     * Report the patterns of a turntable track.
     *
     * @param nTrack The track.
     * @return The patterns.
     * @ghidraAddress NTSC-U/C: 0x0011ee68
     * @ghidraAddress PAL: 0x001205f8
     */
    ScratchTrackData *GetScratchData(int nTrack) const;

    /**
     * Report the notes of a guitar track.
     *
     * @param nTrack The track.
     * @return The notes.
     * @ghidraAddress NTSC-U/C: 0x0011ee88
     * @ghidraAddress PAL: 0x00120618
     */
    AxeTrackData *GetAxeData(int nTrack) const;

    /**
     * Report the riffs of a track.
     *
     * @param nTrack The track.
     * @return The riffs, or null for a track without riffs.
     * @ghidraAddress NTSC-U/C: 0x0011eea8
     * @ghidraAddress PAL: 0x00120638
     */
    PitchTrackRiffData *GetTrackRiffData(int nTrack) const;

    /**
     * Report the gems of a vocal track.
     *
     * @param nTrack The track.
     * @return The gems, or null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x0011eec8
     * @ghidraAddress PAL: 0x00120658
     */
    CatchTrackData *GetVoxData(int nTrack) const;

    /**
     * Report the flags of a track.
     *
     * @param nTrack The track.
     * @return The flags.
     * @ghidraAddress NTSC-U/C: 0x0011eee8
     * @ghidraAddress PAL: 0x00120678
     */
    int GetTrackFlags(int nTrack) const;

    /**
     * Report the number of background music tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0011ef08
     * @ghidraAddress PAL: 0x00120698
     */
    int GetNumBackMusic() const;

    /**
     * Report one background music track.
     *
     * @param nIndex The index.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x0011ef28
     * @ghidraAddress PAL: 0x001206b8
     */
    BackMusic *GetBackMusic(int nIndex) const;

    /**
     * Report the catch track a background music track plays along with.
     *
     * @param nIndex The index of the background music track.
     * @return The catch track, or -1 for music that plays by itself.
     * @ghidraAddress NTSC-U/C: 0x0011ef48
     * @ghidraAddress PAL: 0x001206d8
     */
    int GetBackMusicMaster(int nIndex) const;

    /**
     * Report the number of performances of the tracks named "INTRO".
     *
     * @return The number of performances.
     * @ghidraAddress NTSC-U/C: 0x0011ef60
     * @ghidraAddress PAL: 0x001206f0
     */
    int GetNumIntroMuses() const;

    /**
     * Report the performance of one track named "INTRO".
     *
     * @param nIndex The index.
     * @return The performance.
     * @ghidraAddress NTSC-U/C: 0x0011ef80
     * @ghidraAddress PAL: 0x00120710
     */
    Muse *GetIntroMuse(int nIndex) const;

    /**
     * Find a track named "SCRIPT" of the MIDI file by the name of its commands.
     *
     * @param pszName The name.
     * @return The commands, or null.
     * @ghidraAddress NTSC-U/C: 0x0011efa0
     * @ghidraAddress PAL: 0x00120730
     */
    ScriptTrackData *GetScriptTrack(const char *pszName) const;

    /**
     * Report the sections.
     *
     * @return The sections.
     * @ghidraAddress NTSC-U/C: 0x0011efc0
     * @ghidraAddress PAL: 0x00120750
     */
    SectionBoundaries *GetSections() const;

    /**
     * Report the duration of one tick.
     *
     * @return The duration in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0011efc8
     * @ghidraAddress PAL: 0x00120758
     */
    float *GetMsPerTick() const;

    /**
     * Report the map of the song positions.
     *
     * @return The map.
     * @ghidraAddress NTSC-U/C: 0x0011efd0
     * @ghidraAddress PAL: 0x00120760
     */
    PlayMap *GetPlayMap() const;

    /**
     * Report the track named "BANK".
     *
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x0011efe0
     * @ghidraAddress PAL: 0x00120770
     */
    BankLoader *GetBankTrack() const;

    /**
     * Report the events of the track named "WORLD".
     *
     * @return The events.
     * @ghidraAddress NTSC-U/C: 0x0011eff0
     * @ghidraAddress PAL: 0x00120780
     */
    WorldTrack *GetWorldTrack() const;

    /**
     * Report the lyrics.
     *
     * @return The lyrics.
     * @ghidraAddress NTSC-U/C: 0x0011f000
     * @ghidraAddress PAL: 0x00120790
     */
    Lyric *GetLyric() const;

    /**
     * Report the effect sets.
     *
     * @return The effect sets, or null.
     * @ghidraAddress NTSC-U/C: 0x0011f010
     * @ghidraAddress PAL: 0x001207a0
     */
    FXMgr *GetFXMgr() const;

    /**
     * Report the speed of the song.
     *
     * @return The speed.
     * @ghidraAddress NTSC-U/C: 0x0011f020
     * @ghidraAddress PAL: 0x001207b0
     */
    float GetSpeed() const;

    /**
     * Report the slot grid of the song.
     *
     * @return The patterns of the sections.
     * @ghidraAddress NTSC-U/C: 0x0011f030
     * @ghidraAddress PAL: 0x001207c0
     */
    SlotGrid *GetSlotGrid() const;

    /**
     * Report the gem patterns of a pitch track.
     *
     * The body also calls GetTrackType() and discards the result.
     *
     * @param nTrack The track.
     * @return The kNumPatternFiles patterns of the track, null for another type of track.
     * @ghidraAddress NTSC-U/C: 0x0011f038
     * @ghidraAddress PAL: 0x001207c8
     */
    std::vector<PitchTrackGems *> *GetTrackPitchData(int nTrack);

    /**
     * Report the steps of the catch tracks to enable, each a list of track indices.
     *
     * @return The steps.
     * @ghidraAddress NTSC-U/C: 0x0011f078
     * @ghidraAddress PAL: 0x00120808
     */
    const std::vector<std::vector<int>> &GetEnableOrder() const;

    /**
     * Report whether the song sets the low volumes of a mixer channel.
     *
     * @param nChannel The channel.
     * @return Whether the volumes are set.
     * @ghidraAddress NTSC-U/C: 0x0011f080
     * @ghidraAddress PAL: 0x00120810
     */
    bool HasLowVolumes(int nChannel) const;

    /**
     * Report the low volumes of a mixer channel.
     *
     * The body also calls HasLowVolumes() and discards the result.
     *
     * @param nChannel The channel.
     * @return The volumes.
     * @ghidraAddress NTSC-U/C: 0x0011f0a0
     * @ghidraAddress PAL: 0x00120830
     */
    const std::vector<unsigned char> *GetLowVolumes(int nChannel) const;

    /**
     * Report whether a track has a volume list.
     *
     * @param nTrack The track.
     * @return Whether the volume list of the track is not empty.
     * @ghidraAddress NTSC-U/C: 0x0011f0e0
     * @ghidraAddress PAL: 0x00120870
     */
    bool HasTrackVolumes(int nTrack) const;

    /**
     * Report the volumes of a track.
     *
     * The body also calls HasTrackVolumes() and discards the result.
     *
     * @param nTrack The track.
     * @return The volume list, with the volume of the playing track first and the volume of a
     *         muted track second.
     * @ghidraAddress NTSC-U/C: 0x0011f100
     * @ghidraAddress PAL: 0x00120890
     */
    const std::vector<unsigned char> &GetTrackVolumes(int nTrack) const;

    /**
     * Report whether the song sets the low volumes of the freestyle track.
     *
     * @return Whether the volumes are set.
     * @ghidraAddress NTSC-U/C: 0x0011f140
     * @ghidraAddress PAL: 0x001208d0
     */
    bool HasFreestyleLowVolumes() const;

    /**
     * Report the low volumes of the freestyle track.
     *
     * The body also calls HasFreestyleLowVolumes() and discards the result.
     *
     * @return The volumes.
     * @ghidraAddress NTSC-U/C: 0x0011f158
     * @ghidraAddress PAL: 0x001208e8
     */
    const std::vector<unsigned char> *GetFreestyleLowVolumes() const;

    /**
     * Report the effects bus of a remix.
     *
     * @return `remix_effects_bus`, from 0 to 2.
     * @ghidraAddress NTSC-U/C: 0x0011f180
     * @ghidraAddress PAL: 0x00120910
     */
    int GetRemixEffectsBus() const;

    /**
     * Report the localised name of the effects of a remix.
     *
     * @return The name, or "no name".
     * @ghidraAddress NTSC-U/C: 0x0011f188
     * @ghidraAddress PAL: 0x00120918
     */
    const char *GetRemixEffectsName() const;

    /**
     * Report the label of a remix section.
     *
     * @param nIndex The index of the label.
     * @return The label.
     * @ghidraAddress NTSC-U/C: 0x0011f190
     * @ghidraAddress PAL: 0x00120920
     */
    const char *GetSectionLabel(int nIndex) const;

    int mState;      /*!< The load state, one of LoadState. */
    int mDifficulty; /*!< The difficulty. The duel coaches the players at 1. */
    int mRuleSet;    /*!< The rule set, one of GameDb::RuleSet. */
    TrackBuilder::ErrorHandler mErrorHandler; /*!< The routine that reports an error. */
    int mValidate;                            /*!< Whether errors are reported. */
    int mReserved14;            // +0x14, cleared by the constructor and not otherwise used here.
    LevelMidiBuilder *mBuilder; /*!< The builder of the tracks. */
    DataArray *mConfig;         /*!< The entry of the song. */
    DuelPatternTable mDuelPatterns; /*!< The duel patterns of the song. */
    String mReserved30;           // +0x30, emptied by the constructor and not otherwise used here.
    int mNumBars;                 /*!< `song_bars`, the length of the song in bars. */
    int mIntroBars;               /*!< `intro_bars`, one more than the bars before bar 0. */
    String mMidiFile;             /*!< The path of the MIDI file. */
    String mMidiDir;              /*!< The directory of the MIDI file and the pattern files. */
    SectionBoundaries *mSections; /*!< The sections. */
    float *mMsPerTick;            /*!< The duration of one tick in milliseconds. */
    SlotGrid *mSlotGrid;          /*!< The patterns of the sections, or null. */
    std::vector<int> mBackMusicMasters; /*!< The catch track of each background track, or -1. */
    std::vector<unsigned char> mLowVolumes[kNumMixerChannels]; /*!< `mixer_low_volumes`. */
    std::vector<unsigned char> mFreestyleLowVolumes; /*!< `mixer_low_volumes_freestyle`. */
    std::vector<unsigned char> mTrackVolumes[kNumMixerChannels]; /*!< `duel_mix_volumes`. */
    std::vector<std::vector<int>> mEnableOrder; /*!< The steps of the tracks to enable. */
    SynthEffects mEffects;                      /*!< `effects`, the effects buses. */
    int mFreestyleEffectsBus;                   /*!< `freestyle_effects_bus`. */
    int mRemixEffectsBus;                       /*!< `remix_effects_bus`. */
    const char *mRemixEffectsName;      /*!< The localised `remix_effects_name`, or "no name". */
    std::vector<String> mSectionLabels; /*!< The labels of the remix sections. */
    std::vector<int> mSkippedBars;      /*!< The bars a duel passes over, ascending. */
    std::vector<std::vector<PitchTrackGems *>> mPitchGems; /*!< The patterns of each track. */
    PatternFile *mPatternFiles[kNumPatternFiles]; /*!< The pattern files while they are read. */
    std::vector<int> mDuelTrackOrder; /*!< The tracks a duel plays, cycled by section. */

private:
    /**
     * Read the configuration and start the builder of the MIDI file.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001194d8
     * @ghidraAddress PAL: 0x0011ac70
     */
    void StartLoad();

    /**
     * Report an error in the song text file.
     *
     * @param message The error.
     * @ghidraAddress NTSC-U/C: 0x001197e8
     * @ghidraAddress PAL: 0x0011af78
     */
    void Error(const String &message);

    /**
     * Read a symbol of an entry, reporting it when it is missing.
     *
     * @param pszName The name of the symbol.
     * @param pData The entry.
     * @return The symbol, or null.
     * @ghidraAddress NTSC-U/C: 0x00119890
     * @ghidraAddress PAL: 0x0011b020
     */
    const char *ReadSymbol(const char *pszName, DataArray *pData);

    /**
     * Read an integer of an entry, reporting it when it is missing.
     *
     * @param pszName The name of the integer.
     * @param pData The entry.
     * @return The integer, or 0.
     * @ghidraAddress NTSC-U/C: 0x00119930
     * @ghidraAddress PAL: 0x0011b0c0
     */
    int ReadInt(const char *pszName, DataArray *pData);

    /**
     * Read a number of an entry, reporting it when it is missing.
     *
     * @param pszName The name of the number.
     * @param pData The entry.
     * @return The number, or 0.
     * @ghidraAddress NTSC-U/C: 0x001199d0
     * @ghidraAddress PAL: 0x0011b160
     */
    float ReadFloat(const char *pszName, DataArray *pData);

    /**
     * Find an array of an entry.
     *
     * @param pszName The name of the array.
     * @param pData The entry.
     * @param bRequired Report a missing array.
     * @return The array, or null.
     * @ghidraAddress NTSC-U/C: 0x00119a70
     * @ghidraAddress PAL: 0x0011b200
     */
    DataArray *ReadArray(const char *pszName, DataArray *pData, bool bRequired);

    /**
     * Make the gem patterns of each pitch track, and start reading the pattern files.
     *
     * @ghidraAddress NTSC-U/C: 0x00119b18
     * @ghidraAddress PAL: 0x0011b2a8
     */
    void StartPatternLoad();

    /**
     * Read the length, the tempo, the MIDI file, the mixer, the effects, and the sections.
     *
     * @ghidraAddress NTSC-U/C: 0x0011a5b8
     * @ghidraAddress PAL: 0x0011bd48
     */
    void ReadConfig();

    /**
     * Read the settings of the tracks of a duel or a game once the MIDI file is parsed.
     *
     * @ghidraAddress NTSC-U/C: 0x0011a930
     * @ghidraAddress PAL: 0x0011c0c0
     */
    void ReadTrackConfig();

    /**
     * Read the names, the links, and the labels of the remix sections into mSlotGrid and
     * mSectionLabels.
     *
     * @ghidraAddress NTSC-U/C: 0x0011a990
     * @ghidraAddress PAL: 0x0011c120
     */
    void ReadRemixSections();

    /**
     * Read the first bars of the sections for the rule set.
     *
     * @ghidraAddress NTSC-U/C: 0x0011bda8
     * @ghidraAddress PAL: 0x0011d538
     */
    void ReadSectionStarts();

    /**
     * Make mSections from the first bars of the sections.
     *
     * @param pStarts The first bars, from 1, or -1 for one endless section. Null makes one
     *        section of the song.
     * @ghidraAddress NTSC-U/C: 0x0011be78
     * @ghidraAddress PAL: 0x0011d608
     */
    void BuildSections(DataArray *pStarts);

    /**
     * Read `mixer_overrides` and `mixer_low_volumes_freestyle`.
     *
     * @ghidraAddress NTSC-U/C: 0x0011c420
     * @ghidraAddress PAL: 0x0011dbb0
     */
    void ReadMixerOverrides();

    /**
     * Read `duel_mix_volumes`.
     *
     * @ghidraAddress NTSC-U/C: 0x0011c840
     * @ghidraAddress PAL: 0x0011dfd0
     */
    void ReadDuelMixVolumes();

    /**
     * Read `effects` into mEffects.
     *
     * @ghidraAddress NTSC-U/C: 0x0011cad0
     * @ghidraAddress PAL: 0x0011e260
     */
    void ReadEffects();

    /**
     * Read `enable_order` into mEnableOrder.
     *
     * @ghidraAddress NTSC-U/C: 0x0011cb18
     * @ghidraAddress PAL: 0x0011e2a8
     */
    void ReadEnableOrder();

    /**
     * Read `background_tracks` into mBackMusicMasters.
     *
     * @ghidraAddress NTSC-U/C: 0x0011d128
     * @ghidraAddress PAL: 0x0011e8b8
     */
    void ReadBackgroundTracks();

    /**
     * Read the track order, the tracks without background, and the break bars of a duel.
     *
     * @ghidraAddress NTSC-U/C: 0x0011d378
     * @ghidraAddress PAL: 0x0011eb08
     */
    void ReadDuelTracks();

    /**
     * Read the patterns of a duel at the difficulty into mDuelPatterns.
     *
     * @ghidraAddress NTSC-U/C: 0x0011d970
     * @ghidraAddress PAL: 0x0011f100
     */
    void ReadDuelPatterns();

    /**
     * Report a track enabled twice or a track out of range in mEnableOrder.
     *
     * @ghidraAddress NTSC-U/C: 0x0011e3d8
     * @ghidraAddress PAL: 0x0011fb68
     */
    void ValidateEnableOrder();
};
