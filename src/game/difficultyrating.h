#pragma once

#include <vector>

#include "game/catchtrackdata.h"
#include "game/pitchtrackgems.h"
#include "game/rateaverager.h"

/**
 * Rating of the difficulty of the gems of a remix, from the gaps between consecutive gems.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. A remix rates each of its
 * tracks in turn. Each gap between two gems of a track is worth a difficulty that falls as the gap
 * grows and that is higher when both gems lie in the same lane. The difficulty of a bar is twice
 * the difficulty of the gap from its last gem to its end plus its three hardest gaps. The
 * difficulty of a track is the sum of its gap and bar difficulties over its number of bars with
 * gems.
 */
class DifficultyRating {
public:
    /** Number of sets of bar difficulties the rating records. */
    static constexpr int kNumSlots = 4;

    /**
     * Construct a rating with no tracks rated.
     *
     * @param fTempo The tempo of the remix, in beats per minute.
     * @param nNumBars The number of bars.
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x0013b300
     * @ghidraAddress PAL: 0x0013cbd0
     */
    DifficultyRating(float fTempo, int nNumBars, int nTicksPerBar);

    /**
     * Release the rating.
     *
     * @ghidraAddress NTSC-U/C: 0x0033f098
     * @ghidraAddress PAL: 0x003ac5d0
     */
    ~DifficultyRating() {
    }

    /**
     * Report the difficulty of a gap between two gems.
     *
     * Interpolates the gap in a table of gap lengths.
     *
     * @param nTick The tick of the second gem. The body does not read it.
     * @param bSameLane Both gems lie in the same lane.
     * @param fGapMs The gap, in milliseconds.
     * @return The difficulty.
     * @ghidraAddress NTSC-U/C: 0x0013b440
     * @ghidraAddress PAL: 0x0013cd10
     */
    float GapDifficulty(int nTick, bool bSameLane, float fGapMs) const;

    /**
     * Start rating a track.
     *
     * @param nSlot The set of bar difficulties the track records into.
     * @ghidraAddress NTSC-U/C: 0x0013b4e0
     * @ghidraAddress PAL: 0x0013cdb0
     */
    void BeginTrack(int nSlot);

    /**
     * Rate the next gem of the track, closing the previous bar when the gem starts a new one.
     *
     * @param nLane The lane of the gem.
     * @param nTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x0013b510
     * @ghidraAddress PAL: 0x0013cde0
     */
    void AddGem(int nLane, int nTick);

    /**
     * Finish rating a track and add its mean bar difficulty to the rating.
     *
     * @return Half the mean bar difficulty of the track, truncated.
     * @ghidraAddress NTSC-U/C: 0x0013b848
     * @ghidraAddress PAL: 0x0013d118
     */
    int EndTrack();

    /**
     * Rate the gems of a catch track.
     *
     * @param pData The gems.
     * @param nSlot The set of bar difficulties the track records into.
     * @return The value of EndTrack().
     * @ghidraAddress NTSC-U/C: 0x0013b8c8
     * @ghidraAddress PAL: 0x0013d198
     */
    int RateCatchTrack(const CatchTrackData *pData, int nSlot);

    /**
     * Rate the gems of a pitch track into the first set of bar difficulties.
     *
     * @param pGems The gems.
     * @return The value of EndTrack().
     * @ghidraAddress NTSC-U/C: 0x0013b968
     * @ghidraAddress PAL: 0x0013d238
     */
    int RatePitchTrack(PitchTrackGems *pGems);

    /**
     * Report the skill tier of the rating from the "skill_thresholds" entry of the "game" section
     * of the configuration.
     *
     * @return The number of thresholds the difficulty is not below, 0 to 3.
     * @ghidraAddress NTSC-U/C: 0x0013ba28
     * @ghidraAddress PAL: 0x0013d2f8
     */
    int GetSkillTier();

    /**
     * Report whether the fraction of bars without gems is below the "empty_threshold" entry of the
     * "game" section of the configuration.
     *
     * @return True when the fraction is below the threshold.
     * @ghidraAddress NTSC-U/C: 0x0013bb08
     * @ghidraAddress PAL: 0x0013d3d8
     */
    bool IsBelowEmptyThreshold();

    /**
     * Report the sum of the mean bar difficulties of the rated tracks.
     *
     * @return mDifficulty.
     * @ghidraAddress NTSC-U/C: 0x0013bb80
     * @ghidraAddress PAL: 0x0013d450
     */
    float GetDifficulty() const;

    /**
     * Report the fraction of the bars of the rated tracks without gems.
     *
     * @return The fraction.
     * @ghidraAddress NTSC-U/C: 0x0013bb88
     * @ghidraAddress PAL: 0x0013d458
     */
    float GetEmptyFraction() const;

    float mTempo;                                 /*!< The tempo, in beats per minute. */
    float mMsPerTick;                             /*!< The duration of one tick. */
    int mNumBars;                                 /*!< The number of bars. */
    int mTicksPerBar;                             /*!< The length of a bar in ticks. */
    float mDifficulty;                            /*!< The sum of the track difficulties. */
    int mNumTracks;                               /*!< The number of rated tracks. */
    int mGemBars;                                 /*!< The bars with gems of the tracks. */
    RateAverager mRateAverager;                   /*!< Constructed and not otherwise used. */
    int mSlot;                                    /*!< The slot of the track being rated. */
    int mBar;                                     /*!< The bar of the previous gem, or -1. */
    int mLastLane;                                /*!< The lane of the previous gem, or -1. */
    int mLastTick;                                /*!< The tick of the previous gem. */
    int mTrackGemBars;                            /*!< The bars with gems of the track. */
    std::vector<float> mBarGaps;                  /*!< The gap difficulties of the bar. */
    float mTrackDifficulty;                       /*!< The gap and bar difficulties of the track. */
    std::vector<int> mBarDifficulties[kNumSlots]; /*!< Half of each bar difficulty, by slot. */
};
