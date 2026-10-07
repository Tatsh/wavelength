#pragma once

#include <vector>

#include "gs/scratcher.h"
#include "mid/tickobj.h"

/**
 * The scratchers of a scratch track, one set of three for each span of the song.
 *
 * The class is not polymorphic. The RTTI of the nested ScratcherSet records the name. A set starts
 * at its tick and lasts until the tick of the next set.
 */
class ScratchTrackData {
public:
    /** The number of scratchers in a set. */
    static constexpr int kSetSize = 3;

    /** The scratchers that play from one tick on. */
    struct ScratcherSet {
        Scratcher *mScratchers[kSetSize]; /*!< The scratchers, null where none was given. */
    };

    /**
     * Construct empty data.
     *
     * @param nEndTick The tick at and after which SetScratcher() discards a scratcher.
     * @ghidraAddress NTSC-U/C: 0x00138880
     * @ghidraAddress PAL: 0x0013a0e0
     */
    explicit ScratchTrackData(int nEndTick);

    /**
     * Release every scratcher.
     *
     * @ghidraAddress NTSC-U/C: 0x001388a0
     * @ghidraAddress PAL: 0x0013a100
     */
    ~ScratchTrackData();

    /**
     * Place a scratcher in the set that starts at a tick, creating the set when none starts there.
     *
     * The data then manages the scratcher. A scratcher at or after the end tick is deleted.
     *
     * @param nTick The tick.
     * @param nIndex The position of the scratcher in the set.
     * @param pScratcher The scratcher.
     * @ghidraAddress NTSC-U/C: 0x001389d0
     * @ghidraAddress PAL: 0x0013a230
     */
    void SetScratcher(int nTick, int nIndex, Scratcher *pScratcher);

    /**
     * Report the scratchers of the set that plays at a tick.
     *
     * The set is the last that starts at or before the tick. The data must have one.
     *
     * @param nTick The tick.
     * @param ppFirst Receives the first scratcher of the set.
     * @param ppSecond Receives the second scratcher of the set.
     * @param ppThird Receives the third scratcher of the set.
     * @return The tick of the next set, or -1 when the set is the last.
     * @ghidraAddress NTSC-U/C: 0x00138ac8
     * @ghidraAddress PAL: 0x0013a328
     */
    int
    GetScratchers(int nTick, Scratcher **ppFirst, Scratcher **ppSecond, Scratcher **ppThird) const;

    std::vector<TickObj<ScratcherSet> > mSets; /*!< The sets in ascending tick order. */
    int mEndTick;                              /*!< The end tick SetScratcher() tests. */
};
