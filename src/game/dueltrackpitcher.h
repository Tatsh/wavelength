#pragma once

#include "game/catchtrackdata.h"
#include "game/duelpatterntable.h"
#include "game/pitchtrackriffdata.h"
#include "game/playmap.h"
#include "game/track.h"
#include "gs/muse.h"

/**
 * Places the gems a player presses on a duel track for the other player to catch.
 *
 * The class is not polymorphic. The name comes from the RTTI of its nested class Receiver.
 * DuelTrack embeds one. The presses are rounded to a grid of steps, and a press counts only on a
 * step the duel pattern of the bar allows, in the first part of the phrase. The names of the
 * members are inferred.
 */
class DuelTrackPitcher {
public:
    /**
     * Object told of each press the pitcher judges.
     *
     * The RTTI includes the class name.
     */
    class Receiver {
    public:
        /**
         * Release the receiver.
         *
         * @ghidraAddress NTSC-U/C: 0x003356b0
         * @ghidraAddress PAL: 0x003a2c60
         */
        virtual ~Receiver() {
        }

        /**
         * Handle a press on a step the duel pattern does not allow.
         *
         * @param nTick The tick of the step.
         * @param nSlot The gem button.
         */
        virtual void OnRejected(int nTick, int nSlot) = 0;

        /**
         * Handle a gem placed.
         *
         * @param nTick The tick of the step the gem was placed at.
         */
        virtual void OnPlaced(int nTick) = 0;
    };

    /**
     * Construct a pitcher.
     *
     * @param pTrack The track the presses are made on.
     * @param pReceiver The receiver of the judgements.
     * @param pRiffData The riffs the gems play.
     * @param pGems The gems the other player catches.
     * @param pPlayMap The map of the song positions.
     * @param pPatterns The duel patterns of the song.
     * @param nPhraseTicks The length of a phrase in ticks.
     * @param nCatchSide The side of the player who catches the gems.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nPhraseBars The length of a phrase in bars.
     * @param nWindowTicks The ticks into a phrase in which a press counts.
     * @param bEasiest Whether the song is at the easiest difficulty, at which a press on a step the
     *                 pattern does not allow is ignored.
     * @ghidraAddress NTSC-U/C: 0x0010bdc8
     * @ghidraAddress PAL: 0x0010d500
     */
    DuelTrackPitcher(Track *pTrack,
                     Receiver *pReceiver,
                     PitchTrackRiffData *pRiffData,
                     CatchTrackData *pGems,
                     PlayMap *pPlayMap,
                     DuelPatternTable *pPatterns,
                     int nPhraseTicks,
                     int nCatchSide,
                     int nTicksPerBar,
                     int nPhraseBars,
                     int nWindowTicks,
                     bool bEasiest);

    /**
     * Show the steps the duel pattern allows in a phrase.
     *
     * @param nStartBar The first bar of the phrase.
     * @ghidraAddress NTSC-U/C: 0x0010be50
     * @ghidraAddress PAL: 0x0010d588
     */
    void ShowPattern(int nStartBar);

    /**
     * Judge a press of a gem button at the current tick, and place a gem when the press counts.
     *
     * The gem is sent to the other console in an online duel.
     *
     * @param nSlot The gem button.
     * @ghidraAddress NTSC-U/C: 0x0010bf78
     * @ghidraAddress PAL: 0x0010d6b0
     */
    void Press(int nSlot);

    /**
     * Change the riffs the gems play.
     *
     * @param pRiffData The riffs.
     * @ghidraAddress NTSC-U/C: 0x0010c340
     * @ghidraAddress PAL: 0x0010da78
     */
    void SetRiffData(PitchTrackRiffData *pRiffData);

private:
    Track *mTrack;                 /*!< The track the presses are made on. */
    Receiver *mReceiver;           /*!< The receiver of the judgements. */
    PitchTrackRiffData *mRiffData; /*!< The riffs the gems play. */
    CatchTrackData *mGems;         /*!< The gems the other player catches. */
    PlayMap *mPlayMap;             /*!< The map of the song positions. */
    DuelPatternTable *mPatterns;   /*!< The duel patterns of the song. */
    int mPhraseTicks;              /*!< The length of a phrase in ticks. */
    int mCatchSide;                /*!< The side of the player who catches the gems. */
    int mTicksPerBar;              /*!< The song ticks in one bar. */
    int mPhraseBars;               /*!< The length of a phrase in bars. */
    int mWindowTicks;              /*!< The ticks into a phrase in which a press counts. */
    int mStepTicks;                /*!< The length of a step of the grid in ticks. */
    Muse *mMuse;                   /*!< The riff of the last gem placed, or null. */
    int mLastTick;                 /*!< The catch tick of the last gem placed, or -1. */
    int mLastSlot;                 /*!< The gem button of the last gem placed, or -1. */
    int mRepeats;                  /*!< The gems placed in a row on one button, or -1. */
    int mLockedUntilTick;          /*!< The tick before which every press is refused. */
    int mEasiest;                  /*!< Whether a press the pattern does not allow is ignored. */
};
