#pragma once

#include <vector>

/**
 * Collector of float samples that reports their median.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object records the
 * smallest sample, the largest sample, the sum, and every sample. Only the members GameLogic uses
 * are declared.
 */
class StatsAccumulator {
public:
    /**
     * Construct a collector with no samples.
     *
     * @ghidraAddress NTSC-U/C: 0x002930f0
     * @ghidraAddress PAL: 0x0029cab8
     */
    StatsAccumulator();

    /**
     * Add a sample.
     *
     * @param fSample The sample.
     * @ghidraAddress NTSC-U/C: 0x00293120
     * @ghidraAddress PAL: 0x0029cae8
     */
    void AddSample(float fSample);

    /**
     * Report the median of the samples.
     *
     * @return The median.
     * @ghidraAddress NTSC-U/C: 0x002932f0
     * @ghidraAddress PAL: 0x0029ccb8
     */
    float GetMedian();

    float mMin;                  /*!< The smallest sample. */
    float mMax;                  /*!< The largest sample. */
    float mSum;                  /*!< The sum of the samples. */
    std::vector<float> mSamples; /*!< Every sample. */
};
