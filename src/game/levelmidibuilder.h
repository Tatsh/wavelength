#pragma once

#include "game/axecontour.h"
#include "game/backmusic.h"
#include "game/banktrack.h"
#include "game/catchtrackdata.h"
#include "game/freestylefx.h"
#include "game/lyric.h"
#include "game/playmap.h"
#include "game/scratchtrackdata.h"
#include "game/worldtrack.h"
#include "gs/muse.h"

/**
 * Reader of the MIDI file of a song, which builds the data of every track it names.
 *
 * The RTTI records the class as deriving from MidiReceiver. Only the members Song and GameLogic
 * use are declared.
 */
class LevelMidiBuilder {
public:
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
    AxeContour *GetAxeContour(int nTrack) const;

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

    PlayMap *mPlayMap;         /*!< The map of the song positions. +0x40 */
    BankTrack *mBankTrack;     /*!< The track named "BANK". +0x44 */
    WorldTrack mWorldTrack;    /*!< The events of the track named "WORLD". +0x48 */
    Lyric *mLyric;             /*!< The lyrics. +0x58 */
    FreestyleFx *mFreestyleFx; /*!< The effect sets, or null. +0x5c */
    float mSpeed;              /*!< The speed of the song. +0x60 */
    int mTicksPerBar;          /*!< The length of a bar in ticks. +0x98 */
};
