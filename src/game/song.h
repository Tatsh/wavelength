#pragma once

#include <vector>

#include "game/axecontour.h"
#include "game/backmusic.h"
#include "game/banktrack.h"
#include "game/catchtrackdata.h"
#include "game/duelpatterntable.h"
#include "game/freestylefx.h"
#include "game/levelmidibuilder.h"
#include "game/lyric.h"
#include "game/pitchtrackgems.h"
#include "game/pitchtrackriffdata.h"
#include "game/playmap.h"
#include "game/scratchtrackdata.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "game/worldtrack.h"
#include "gs/muse.h"

/**
 * One song: its configuration from the song text file and the tracks its MIDI file builds.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the song text file it
 * reads. Only the members its callers here use are declared.
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

    /** Number of mixer channels the song can set the low volumes of. */
    static constexpr int kNumMixerChannels = 16;

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
     */
    CatchTrackData *GetCatchTrackData(int nTrack) const;

    /**
     * Report the patterns of a turntable track.
     *
     * @param nTrack The track.
     * @return The patterns.
     * @ghidraAddress NTSC-U/C: 0x0011ee68
     */
    ScratchTrackData *GetScratchData(int nTrack) const;

    /**
     * Report the notes of a guitar track.
     *
     * @param nTrack The track.
     * @return The notes.
     * @ghidraAddress NTSC-U/C: 0x0011ee88
     */
    AxeContour *GetAxeContour(int nTrack) const;

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
     * Report the flags of a track.
     *
     * @param nTrack The track.
     * @return The flags.
     * @ghidraAddress NTSC-U/C: 0x0011eee8
     */
    int GetTrackFlags(int nTrack) const;

    /**
     * Report the number of background music tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0011ef08
     */
    int GetNumBackMusic() const;

    /**
     * Report one background music track.
     *
     * @param nIndex The index.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x0011ef28
     */
    BackMusic *GetBackMusic(int nIndex) const;

    /**
     * Report the catch track a background music track plays along with.
     *
     * @param nIndex The index of the background music track.
     * @return The catch track, or -1 for music that plays by itself.
     * @ghidraAddress NTSC-U/C: 0x0011ef48
     */
    int GetBackMusicMaster(int nIndex) const;

    /**
     * Report the number of performances of the tracks named "INTRO".
     *
     * @return The number of performances.
     * @ghidraAddress NTSC-U/C: 0x0011ef60
     */
    int GetNumIntroMuses() const;

    /**
     * Report the performance of one track named "INTRO".
     *
     * @param nIndex The index.
     * @return The performance.
     * @ghidraAddress NTSC-U/C: 0x0011ef80
     */
    Muse *GetIntroMuse(int nIndex) const;

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
     */
    BankTrack *GetBankTrack() const;

    /**
     * Report the events of the track named "WORLD".
     *
     * @return The events.
     * @ghidraAddress NTSC-U/C: 0x0011eff0
     */
    WorldTrack *GetWorldTrack() const;

    /**
     * Report the lyrics.
     *
     * @return The lyrics.
     * @ghidraAddress NTSC-U/C: 0x0011f000
     */
    Lyric *GetLyric() const;

    /**
     * Report the effect sets.
     *
     * @return The effect sets, or null.
     * @ghidraAddress NTSC-U/C: 0x0011f010
     */
    FreestyleFx *GetFreestyleFx() const;

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
     * @return The grid the song refers to at `+0x7c`.
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
     * @return The entry of the track in the array at `+0x348`.
     * @ghidraAddress NTSC-U/C: 0x0011f038
     * @ghidraAddress PAL: 0x001207c8
     */
    std::vector<PitchTrackGems *> *GetTrackPitchData(int nTrack);

    /**
     * Report the steps of the catch tracks to enable, each a list of track indices.
     *
     * @return The steps.
     * @ghidraAddress NTSC-U/C: 0x0011f078
     */
    const std::vector<std::vector<int>> &GetEnableOrder() const;

    /**
     * Report whether the song sets the low volumes of a mixer channel.
     *
     * @param nChannel The channel.
     * @return Whether the volumes are set.
     * @ghidraAddress NTSC-U/C: 0x0011f080
     */
    bool HasLowVolumes(int nChannel) const;

    /**
     * Report the low volumes of a mixer channel.
     *
     * @param nChannel The channel.
     * @return The volumes.
     * @ghidraAddress NTSC-U/C: 0x0011f0a0
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
     */
    bool HasFreestyleLowVolumes() const;

    /**
     * Report the low volumes of the freestyle track.
     *
     * @return The volumes.
     * @ghidraAddress NTSC-U/C: 0x0011f158
     */
    const std::vector<unsigned char> *GetFreestyleLowVolumes() const;

    unsigned char mReserved00[0x04]; // +0x00, not yet recovered.
    int mDifficulty; /*!< The difficulty. The duel coaches the players at 1. +0x04 */
    unsigned char mReserved08[0x10]; // +0x08, not yet recovered.
    LevelMidiBuilder *mBuilder;      /*!< The builder of the tracks. +0x18 */
    unsigned char mReserved1c[0x04]; // +0x1c, not yet recovered.
    DuelPatternTable mDuelPatterns;  /*!< The duel patterns of the song. +0x20 */
    unsigned char mReserved30[0x14]; // +0x30, not yet recovered.
    int mNumBars;                    /*!< `song_bars`, the length of the song in bars. +0x44 */
    int mIntroBars; /*!< `intro_bars`, one more than the bars before bar 0. +0x48 */
    unsigned char mReserved4c[0x2ec]; // +0x4c, not yet recovered.
    std::vector<int> mSkippedBars;    /*!< The bars a duel passes over, ascending. +0x338 */
    unsigned char mReserved348[0x1c]; // +0x348, not yet recovered.
    std::vector<int> mDuelTrackOrder; /*!< The tracks a duel plays, cycled by section. +0x364 */
};
