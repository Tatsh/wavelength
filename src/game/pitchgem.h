#pragma once

/**
 * One gem of a pitch track.
 *
 * The RTTI includes the class name. The class is not polymorphic.
 */
class PitchGem {
public:
    PitchGem() = default;

    /**
     * Construct a gem.
     *
     * @param nSlot The gem button the gem lies under.
     * @param nTick The tick of the gem.
     * @param nOwner The player who placed the gem, or -1.
     */
    PitchGem(signed char nSlot, int nTick, signed char nOwner)
        : mSlot(nSlot), mTick(nTick), mOwner(nOwner) {
    }

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

    /** @return The tick of the gem. */
    int GetTick() const {
        return mTick;
    }

    /** @param nTick The tick of the gem. */
    void SetTick(int nTick) {
        mTick = nTick;
    }

    /** @return The gem button the gem lies under. */
    signed char GetSlot() const {
        return mSlot;
    }

    /** @return The player who placed the gem, or -1. */
    signed char GetOwner() const {
        return mOwner;
    }

    /** @param nOwner The player who placed the gem, or -1. */
    void SetOwner(signed char nOwner) {
        mOwner = nOwner;
    }

private:
    signed char mSlot;  /*!< The gem button the gem lies under. */
    int mTick;          /*!< The tick of the gem. */
    signed char mOwner; /*!< The player who placed the gem, or -1. */
};
