#pragma once

#include "game/gemslice.h"
#include "game/pitchtrackplaygems.h"
#include "game/pitchtrackriffs.h"
#include "gs/muse.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Player of the riff of each gem of a pitch track at the gem's tick.
 *
 * The RTTI records the commands the class schedules. The class is not polymorphic. The member
 * names are inferred.
 */
class PitchTrackMusic {
public:
    /**
     * Construct a player of gems.
     *
     * @param pGems The gems.
     * @param pRiffs The riffs of the gems.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x0012d238
     * @ghidraAddress PAL: 0x0012ea10
     */
    PitchTrackMusic(PitchTrackPlayGems *pGems, PitchTrackRiffs *pRiffs, int nTicksPerBar);

    /**
     * Withdraw the scheduled commands and release the player.
     *
     * @ghidraAddress NTSC-U/C: 0x0012d2d8
     * @ghidraAddress PAL: 0x0012eab0
     */
    ~PitchTrackMusic();

    /**
     * Schedule the first bar at tick 0.
     *
     * @ghidraAddress NTSC-U/C: 0x0012d340
     * @ghidraAddress PAL: 0x0012eb18
     */
    void Start();

    /**
     * Withdraw the scheduled commands.
     *
     * @ghidraAddress NTSC-U/C: 0x0012d380
     * @ghidraAddress PAL: 0x0012eb58
     */
    void Stop();

    /**
     * Play other gems from the current bar on.
     *
     * @param pGems The gems.
     * @ghidraAddress NTSC-U/C: 0x0012d3d8
     * @ghidraAddress PAL: 0x0012ebb0
     */
    void SetGems(PitchTrackPlayGems *pGems);

    /**
     * Stop the riff that plays and play another.
     *
     * @param pRiff The riff.
     * @param nOffset The ticks the riff's events move by.
     * @ghidraAddress NTSC-U/C: 0x0012d440
     * @ghidraAddress PAL: 0x0012ec18
     */
    void Play(Muse *pRiff, int nOffset);

    /**
     * Set whether the gems play, stopping the riff that plays when they no longer do.
     *
     * @param bMuted Whether the gems are silent.
     * @ghidraAddress NTSC-U/C: 0x0012d4b8
     * @ghidraAddress PAL: 0x0012ec90
     */
    void SetMuted(bool bMuted);

private:
    /**
     * Read the gems of the current bar and schedule the first gem still ahead, then schedule the
     * next bar.
     *
     * @ghidraAddress NTSC-U/C: 0x0012d4f8
     * @ghidraAddress PAL: 0x0012ecd0
     */
    void StartBar();

    /**
     * Play the riff of the gem at the current tick, then schedule the next gem of the bar.
     *
     * @ghidraAddress NTSC-U/C: 0x0012d700
     * @ghidraAddress PAL: 0x0012eed8
     */
    void PlayGem();

    int mTicksPerBar;             /*!< The song ticks in one bar. */
    PitchTrackPlayGems *mGems;    /*!< The gems. */
    PitchTrackRiffs *mRiffs;      /*!< The riffs of the gems. */
    GemSlice mBar;                /*!< The gems of the current bar. */
    int mNextGem;                 /*!< The index of the next gem of the bar. */
    Ptr<Command> mPlayGemCommand; /*!< The command that calls PlayGem(). */
    Ptr<Command> mBarCommand;     /*!< The command that calls StartBar(). */
    Muse *mPlaying;               /*!< The riff that plays, or null. */
    bool mMuted;                  /*!< Whether the gems are silent. */
};
