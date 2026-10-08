#pragma once

#include "gfx/ship.h"
#include "math/interpolator.h"
#include "rnd/transanim.h"
#include "rnd/transformable.h"

/**
 * Flight of the ship of the tutorial onto the tracks, `ship tutorial fly-in.tnm`.
 *
 * The class emits no RTTI, and the name is inferred. The object is 0x28 bytes. The ship stays
 * hidden until the first SetShown() that shows it, which starts the flight over the next bar.
 */
class ShipFlyIn {
public:
    /**
     * Construct a flight that has not started and hide the ship.
     *
     * @param pShip The ship.
     * @ghidraAddress NTSC-U/C: 0x001cb200
     * @ghidraAddress PAL: 0x001d3fa0
     */
    explicit ShipFlyIn(Ship *pShip);

    /**
     * Destroy the flight.
     *
     * @ghidraAddress NTSC-U/C: 0x001cb2b0
     * @ghidraAddress PAL: 0x001d4050
     */
    ~ShipFlyIn() = default;

    /**
     * Apply the flight to the transform of the ship.
     *
     * @param fTick The tick of the song.
     * @param fTime The time of the song. The flight does not read it.
     * @param pXfm The transform of the ship, which the flight is applied to.
     * @return Whether the flight is over.
     * @ghidraAddress NTSC-U/C: 0x001cb2e0
     * @ghidraAddress PAL: 0x001d4080
     */
    bool Poll(float fTick, float fTime, float (*pXfm)[Rnd::kXfmRowFloatCount]);

    /**
     * Start the flight the first time the ship is shown.
     *
     * @param bShow Whether the ship is to be shown.
     * @return bShow.
     * @ghidraAddress NTSC-U/C: 0x001cb360
     * @ghidraAddress PAL: 0x001d4100
     */
    bool Start(bool bShow);

    Ship *mShip;              /*!< The ship. */
    int mStarted;             /*!< Whether the flight has started. */
    LinearInterpolator mPath; /*!< The frame of the flight over the ticks of the song. */
    Rnd::TransAnim *mAnim;    /*!< `ship tutorial fly-in.tnm`. */
};
