#pragma once

/**
 * One gem of a pitch track.
 *
 * The RTTI includes the type name. The type is not polymorphic.
 */
struct PitchGem {
    signed char mSlot;  /*!< The gem button the gem lies under. */
    int mTick;          /*!< The tick of the gem. */
    signed char mOwner; /*!< The player who placed the gem, or -1. */
};
