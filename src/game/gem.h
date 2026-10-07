#pragma once

#include "gs/muse.h"
#include "os/ptr.h"

/**
 * One gem of a catch track.
 *
 * The RTTI includes the class name, and CatchTrackData keeps the gems of a track in a vector
 * ordered by mTick. The object is 0x0c bytes.
 */
struct Gem {
    int mLane;       /*!< The lane of the gem. */
    int mTick;       /*!< The tick of the gem in the song as written. */
    Ptr<Muse> mMuse; /*!< The music the gem plays when caught. */
};
