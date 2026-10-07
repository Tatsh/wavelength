#pragma once

#include <vector>

#include "game/axecontour.h"
#include "game/backmusic.h"
#include "game/banktrack.h"
#include "game/catchtrackdata.h"
#include "game/freestylefx.h"
#include "game/levelmidibuilder.h"
#include "game/lyric.h"
#include "game/playmap.h"
#include "game/scratchdata.h"
#include "game/sectionlist.h"
#include "game/worldtrack.h"
#include "gs/muse.h"

/**
 * One song: its configuration from the song text file and the tracks its MIDI file builds.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the song text file it
 * reads. Only the members GameLogic uses are declared.
 */
class Song {
public:
    /** Types of an instrument track. */
    enum TrackType {
        kTrackTypeAxe = 1,     /*!< A guitar freestyle track. */
        kTrackTypeVox = 2,     /*!< A vocal track. */
        kTrackTypeScratch = 3, /*!< A turntable freestyle track. */
        kTrackTypePitch = 4,   /*!< A pitch track. */
        kTrackTypeCatch = 5,   /*!< A catch track. */
    };

    /** Number of mixer channels the song can set the low volumes of. */
    static constexpr int kNumMixerChannels = 16;

    /**
     * Report the number of instrument tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0011ede0
     */
    int GetNumTracks() const;

    /**
     * Report the type of an instrument track.
     *
     * @param nTrack The track.
     * @return One of TrackType.
     * @ghidraAddress NTSC-U/C: 0x0011ee00
     */
    int GetTrackType(int nTrack) const;

    /**
     * Report the instrument of a track.
     *
     * @param nTrack The track.
     * @return The instrument.
     * @ghidraAddress NTSC-U/C: 0x0011ee20
     */
    int GetTrackInstrument(int nTrack) const;

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
    ScratchData *GetScratchData(int nTrack) const;

    /**
     * Report the notes of a guitar track.
     *
     * @param nTrack The track.
     * @return The notes.
     * @ghidraAddress NTSC-U/C: 0x0011ee88
     */
    AxeContour *GetAxeContour(int nTrack) const;

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
     */
    SectionList *GetSections() const;

    /**
     * Report the duration of one tick.
     *
     * @return The duration in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0011efc8
     */
    float *GetMsPerTick() const;

    /**
     * Report the map of the song positions.
     *
     * @return The map.
     * @ghidraAddress NTSC-U/C: 0x0011efd0
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
     */
    float GetSpeed() const;

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

    LevelMidiBuilder *mBuilder; /*!< The builder of the tracks. +0x18 */
    int mNumBars;               /*!< `song_bars`, the length of the song in bars. +0x44 */
    int mIntroBars;             /*!< `intro_bars`, the bars before the first section. +0x48 */
};
