#pragma once

/**
 * One gem of a pitch track.
 *
 * The RTTI includes the class name. The class is not polymorphic.
 */
class PitchGem {
public:
    /**
     * Order gems by tick.
     *
     * Inline. The searches of a bar's gems expand it.
     *
     * @param other The other gem.
     * @return Whether this gem comes first.
     */
    bool operator<(const PitchGem &other) const {
        return mTick < other.mTick;
    }

    signed char mSlot;  /*!< The gem button the gem lies under. */
    int mTick;          /*!< The tick of the gem. */
    signed char mOwner; /*!< The player who placed the gem, or -1. */
};
