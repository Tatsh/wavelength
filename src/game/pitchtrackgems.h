#pragma once

#include <vector>

#include "game/pitchgem.h"

/**
 * The gems of a pitch track, by the bar of the song as written they lie in.
 *
 * The class is not polymorphic. The name comes from the RTTI of its nested class SecStats. Each bar
 * keeps its gems in tick order. Only the members its callers here use are declared.
 */
class PitchTrackGems {
public:
    /**
     * Report the gems of a bar.
     *
     * @param nBar The bar.
     * @return The gems.
     * @ghidraAddress NTSC-U/C: 0x0012bdf0
     * @ghidraAddress PAL: 0x0012d5e8
     */
    std::vector<PitchGem> *GetBar(int nBar);
};
