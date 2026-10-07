#pragma once

#include "game/catchtrackstate.h"
#include "game/gemcursor.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Player of the guide sound of each gem of a catch track as the song reaches the gem.
 *
 * The RTTI records only the command the class schedules. The class is not polymorphic. CatchTrack
 * builds one. Gems of bars that are disabled or already captured play no sound, and the ticker can
 * be muted, indefinitely or until two more bars with gems have passed.
 */
class GuideTicker {
public:
    /**
     * Construct a ticker at the first gem of the track.
     *
     * @param pState The state of the bars of the track.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x00150f90
     * @ghidraAddress PAL: 0x00152880
     */
    GuideTicker(CatchTrackState *pState, int nTicksPerBar);

    /**
     * Withdraw the scheduled gem.
     *
     * @ghidraAddress NTSC-U/C: 0x00151008
     * @ghidraAddress PAL: 0x001528f8
     */
    ~GuideTicker();

    /**
     * Schedule an update for the current tick.
     *
     * @ghidraAddress NTSC-U/C: 0x00151058
     * @ghidraAddress PAL: 0x00152948
     */
    void Start();

    /**
     * Withdraw the scheduled update.
     *
     * @ghidraAddress NTSC-U/C: 0x00151098
     * @ghidraAddress PAL: 0x00152988
     */
    void Stop();

    /**
     * Mute the ticker until Unmute().
     *
     * @ghidraAddress NTSC-U/C: 0x001510d0
     * @ghidraAddress PAL: 0x001529c0
     */
    void Mute();

    /**
     * Let the ticker play again.
     *
     * @ghidraAddress NTSC-U/C: 0x001510e0
     * @ghidraAddress PAL: 0x001529d0
     */
    void Unmute();

    /**
     * Mute the ticker until the gems of two bars after the current bar have passed.
     *
     * @ghidraAddress NTSC-U/C: 0x001510e8
     * @ghidraAddress PAL: 0x001529d8
     */
    void MuteForTwoBars();

    /**
     * Play the gems the song has reached and schedule the next gem.
     *
     * @ghidraAddress NTSC-U/C: 0x00151138
     * @ghidraAddress PAL: 0x00152a28
     */
    void Update();

private:
    int mTicksPerBar;        /*!< The song ticks in one bar. */
    int mLastBar;            /*!< The bar of the last gem counted while muted, or -1. */
    int mBarsLeft;           /*!< The bars left to count before the ticker unmutes, or 0. */
    int mMuted;              /*!< Whether the ticker is muted. */
    CatchTrackState *mState; /*!< The state of the bars of the track. */
    GemCursor mCursor;       /*!< The next gem. */
    Ptr<Command> mUpdateCmd; /*!< The command that calls Update(). */
};
