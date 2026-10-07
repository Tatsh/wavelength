#pragma once

#include <vector>

#include "game/gem.h"
#include "game/playmap.h"

/**
 * The gems of one catch track, in the order of their ticks.
 *
 * The RTTI includes the class name. The class is not polymorphic. The object is 0x14 bytes. Gems
 * at the same tick are in the order of their lanes.
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
     * Construct an empty list with room for 500 gems and no length.
     *
     * @ghidraAddress NTSC-U/C: 0x0014bca8
     * @ghidraAddress PAL: 0x0014d648
     */
    CatchTrackData();

    /**
     * Release the gems.
     *
     * @ghidraAddress NTSC-U/C: 0x0014bce8
     * @ghidraAddress PAL: 0x0014d688
     */
    ~CatchTrackData();

    /**
     * Insert a gem at the place its tick orders it, unless it is past the end of the track.
     *
     * The name is inferred.
     *
     * @param gem The gem.
     * @ghidraAddress NTSC-U/C: 0x0014bd90
     * @ghidraAddress PAL: 0x0014d730
     */
    void AddGem(const Gem &gem);

    /**
     * Remove the gems of a span of the song, which wraps past the end of the track when it ends
     * before it starts.
     *
     * The name is inferred.
     *
     * @param pPlayMap The map of the song positions.
     * @param nStartTick The first song tick of the span.
     * @param nEndTick The song tick after the span.
     * @ghidraAddress NTSC-U/C: 0x0014c028
     * @ghidraAddress PAL: 0x0014d9c8
     */
    void EraseSpan(PlayMap *pPlayMap, int nStartTick, int nEndTick);

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
    /**
     * Walk the gems. The body does nothing else.
     *
     * @ghidraAddress NTSC-U/C: 0x0014c470
     * @ghidraAddress PAL: 0x0014de10
     */
    void CheckGems() const;

    std::vector<Gem> mGems; /*!< The gems in the order of their ticks. */
    int mLengthTicks;       /*!< The length of the track in ticks, or -1. */
};
