#pragma once

#include <vector>

#include "game/pitchgem.h"

/**
 * View of the gems of one bar of a pitch track's song as written, moved to a bar the song plays.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Its members are out of line,
 * and only the ones its callers here use are declared.
 */
class GemSlice {
public:
    /**
     * Construct a view of no gems.
     *
     * @ghidraAddress NTSC-U/C: 0x00151a10
     * @ghidraAddress PAL: 0x00153258
     */
    GemSlice();

    /**
     * Construct a view of the gems of a bar.
     *
     * @param pGems The gems.
     * @param nOffset The ticks added to the tick of each gem.
     * @ghidraAddress NTSC-U/C: 0x00151a20
     * @ghidraAddress PAL: 0x00153268
     */
    GemSlice(const std::vector<PitchGem> *pGems, int nOffset);

    /**
     * Release the view.
     *
     * @ghidraAddress NTSC-U/C: 0x00151a30
     * @ghidraAddress PAL: 0x00153278
     */
    ~GemSlice();

    /**
     * Copy another view.
     *
     * @param other The view.
     * @return This view.
     * @ghidraAddress NTSC-U/C: 0x00151a58
     * @ghidraAddress PAL: 0x001532a0
     */
    GemSlice &operator=(const GemSlice &other);

    /**
     * Report the number of gems.
     *
     * Inline.
     *
     * @return The number of gems, or 0 for a view of no gems.
     */
    int Size() const {
        return mGems != nullptr ? static_cast<int>(mGems->size()) : 0;
    }

    /**
     * Report the gem at an index.
     *
     * Inline. A bad index reports an error and stops the program.
     *
     * @param nIndex The index.
     * @return The gem.
     */
    const PitchGem &At(int nIndex) const {
        return mGems->at(nIndex);
    }

    /**
     * Report the tick of the gem at an index, moved to the bar the song plays.
     *
     * Inline.
     *
     * @param nIndex The index.
     * @return The tick.
     */
    int TickAt(int nIndex) const {
        return At(nIndex).mTick + mOffset;
    }

    const std::vector<PitchGem> *mGems; /*!< The gems, or null. */
    int mOffset;                        /*!< The ticks added to the tick of each gem. */
};
