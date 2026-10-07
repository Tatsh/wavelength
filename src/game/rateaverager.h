#pragma once

/**
 * Average of the rate of the gems of one bar.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each sample after the
 * first adds the length of a bar divided by the ticks since the previous sample.
 */
class RateAverager {
public:
    /**
     * Construct an average with no samples.
     *
     * @param nTicksPerBar The length of a bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x0014a8b0
     * @ghidraAddress PAL: 0x0014c270
     */
    explicit RateAverager(int nTicksPerBar);

    /**
     * Forget every sample.
     *
     * @ghidraAddress NTSC-U/C: 0x0014a8e0
     * @ghidraAddress PAL: 0x0014c2a0
     */
    void Reset();

    /**
     * Add a gem.
     *
     * @param nSlot The slot of the gem. The body does not read it.
     * @param nTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x0014a8f8
     * @ghidraAddress PAL: 0x0014c2b8
     */
    void Sample(int nSlot, int nTick);

    /**
     * Report the average rate.
     *
     * @return The sum of the rates over the number of samples.
     * @ghidraAddress NTSC-U/C: 0x0014a948
     * @ghidraAddress PAL: 0x0014c308
     */
    float GetMean() const;

    int mTicksPerBar; /*!< The length of a bar in ticks. */
    float mSum;       /*!< The sum of the rates. */
    int mCount;       /*!< The number of samples. */
    int mLastTick;    /*!< The tick of the previous sample, or -1. */
};
