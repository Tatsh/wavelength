#pragma once

#include <vector>

#include "game/gem.h"

/**
 * The gems of one catch track, in the order of their ticks.
 *
 * The RTTI includes the class name. The class is not polymorphic. The object is 0x14 bytes. Only
 * the members its callers here use are declared.
 */
class CatchTrackData {
public:
    /**
     * Construct an empty list with room for 500 gems.
     *
     * @param nLengthTicks The length of the track in ticks.
     * @ghidraAddress NTSC-U/C: 0x0014bc68
     * @ghidraAddress PAL: 0x0014d608
     */
    explicit CatchTrackData(int nLengthTicks);

    /**
     * Release the gems.
     *
     * @ghidraAddress NTSC-U/C: 0x0014bce8
     * @ghidraAddress PAL: 0x0014d688
     */
    ~CatchTrackData();

    /**
     * Insert a gem at the place its tick orders it.
     *
     * The name is inferred.
     *
     * @param gem The gem.
     * @ghidraAddress NTSC-U/C: 0x0014bd90
     * @ghidraAddress PAL: 0x0014d730
     */
    void AddGem(const Gem &gem);

    /**
     * Report the number of gems.
     *
     * @return The number of gems.
     * @ghidraAddress NTSC-U/C: 0x0014c298
     * @ghidraAddress PAL: 0x0014dc38
     */
    int GetNumGems() const;

    /**
     * Report one gem.
     *
     * @param nIndex The index of the gem.
     * @return The gem.
     * @ghidraAddress NTSC-U/C: 0x0014c2b8
     * @ghidraAddress PAL: 0x0014dc58
     */
    Gem *GetGem(int nIndex);

    /**
     * Report one gem.
     *
     * @param nIndex The index of the gem.
     * @return The gem.
     * @ghidraAddress NTSC-U/C: 0x0014c2d0
     * @ghidraAddress PAL: 0x0014dc70
     */
    const Gem *GetGem(int nIndex) const;

    /**
     * Find the gem at a tick.
     *
     * The name is inferred.
     *
     * @param nTick The tick in the song as written.
     * @return The index of the gem, or -1 when no gem lies at the tick.
     * @ghidraAddress NTSC-U/C: 0x0014c2f0
     * @ghidraAddress PAL: 0x0014dc90
     */
    int FindGemAt(int nTick) const;

    /**
     * Find the first gem at or after a tick.
     *
     * @param nTick The tick in the song as written.
     * @return The index of the gem, or -1 when every gem is earlier.
     * @ghidraAddress NTSC-U/C: 0x0014c3b8
     * @ghidraAddress PAL: 0x0014dd58
     */
    int FindGem(int nTick) const;

private:
    std::vector<Gem> mGems; /*!< The gems in the order of their ticks. */
    int mLengthTicks;       /*!< The length of the track in ticks. */
};
