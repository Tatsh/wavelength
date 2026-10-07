#pragma once

#include "gs/muse.h"
#include "os/ptr.h"

/**
 * The gems of one catch track, in the order of their ticks.
 *
 * The RTTI includes the class name. The class is not polymorphic. Only the members its callers here
 * use are declared.
 */
class CatchTrackData {
public:
    /**
     * One gem of the track.
     *
     * The class has no RTTI. The name is inferred.
     */
    struct Gem {
        int mLane;       /*!< The lane of the gem. */
        int mTick;       /*!< The tick of the gem in the song as written. */
        Ptr<Muse> mMuse; /*!< The music the gem plays when caught. */
    };

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
     * Find the first gem at or after a tick.
     *
     * @param nTick The tick in the song as written.
     * @return The index of the gem, or -1 when every gem is earlier.
     * @ghidraAddress NTSC-U/C: 0x0014c3b8
     * @ghidraAddress PAL: 0x0014dd58
     */
    int FindGem(int nTick);
};
