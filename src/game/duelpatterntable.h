#pragma once

#include <algorithm>
#include <vector>

#include "game/duelpattern.h"

/**
 * The duel patterns of a song, each with the tick it applies from, embedded in the song at
 * `+0x20`.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred.
 */
class DuelPatternTable {
public:
    /** One pattern and the tick it applies from. The name is inferred. */
    struct Entry {
        int mTick;            /*!< The tick the pattern applies from. */
        DuelPattern mPattern; /*!< The pattern. */
    };

    /**
     * Find the pattern that applies at a tick.
     *
     * Inline. The emitted copy is at the address below.
     *
     * @param nTick The tick.
     * @return The last entry that starts at or before the tick, or the end of mEntries when the
     *         tick precedes every entry. DuelTrackPitcher reads the end as an entry.
     * @ghidraAddress NTSC-U/C: 0x003357b0
     * @ghidraAddress PAL: 0x003a2d60
     */
    std::vector<Entry>::const_iterator Find(int nTick) const {
        const auto it = std::upper_bound(
            mEntries.begin(), mEntries.end(), nTick, [](int nKey, const Entry &entry) {
                return nKey < entry.mTick;
            });
        if (it == mEntries.begin()) {
            return mEntries.end();
        }
        return it - 1;
    }

    std::vector<Entry> mEntries; /*!< The patterns in the order of their ticks. */
};
