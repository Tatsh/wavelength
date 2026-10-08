#pragma once

#include <list>

#include "gfx/beamparams.h"
#include "gfx/ship.h"
#include "math/color.h"
#include "math/vector3.h"
#include "rnd/drawable.h"

/**
 * Beam a ship fires at a gem.
 *
 * The RTTI names the class through the beam pools. The object is 0x70 bytes. Only the members the
 * ships and the pools use are declared so far.
 */
class Beam {
public:
    /**
     * Construct an idle beam.
     *
     * @ghidraAddress NTSC-U/C: 0x001ed2d0
     * @ghidraAddress PAL: 0x001f6070
     */
    Beam();

    /**
     * Destroy the beam.
     *
     * @ghidraAddress NTSC-U/C: 0x001ed458
     * @ghidraAddress PAL: 0x001f61f8
     */
    ~Beam();

    /**
     * Take the look of a family of beams.
     *
     * @param pParams The look.
     * @ghidraAddress NTSC-U/C: 0x001ed4e0
     * @ghidraAddress PAL: 0x001f6280
     */
    void Configure(const BeamParams *pParams);

    /**
     * Fire the beam for a ship.
     *
     * @param pOwner The ship.
     * @param pOwnerList The list of the ship that records the beam.
     * @param fStart The time the beam starts.
     * @param fEnd The time the beam ends.
     * @ghidraAddress NTSC-U/C: 0x001ed510
     * @ghidraAddress PAL: 0x001f62b0
     */
    void Start(Ship *pOwner, std::list<Ship::BeamData> *pOwnerList, float fStart, float fEnd);

    /**
     * Stop the beam.
     *
     * @ghidraAddress NTSC-U/C: 0x001ed5c0
     * @ghidraAddress PAL: 0x001f6360
     */
    void Stop();

    /**
     * Advance the beam.
     *
     * @param direction The direction the beam leaves the ship in.
     * @param side The direction across the beam.
     * @param color The colour of the ship.
     * @return Whether the beam has ended.
     * @ghidraAddress NTSC-U/C: 0x001ed608
     * @ghidraAddress PAL: 0x001f63a8
     */
    bool Update(const Vector3 &direction, const Vector3 &side, const Color &color);

    /**
     * Move one end of the beam.
     *
     * @param nEnd The end, 0 at the ship and 1 at the gem.
     * @param pPosition The position, four floats.
     * @ghidraAddress NTSC-U/C: 0x001ed9c0
     * @ghidraAddress PAL: 0x001f6760
     */
    void SetEnd(int nEnd, const float *pPosition);

    unsigned char mReserved00[0x30];       // +0x00, not yet recovered.
    Rnd::Drawable *mDraw;                  /*!< What draws the beam. +0x30 */
    unsigned char mReserved34[0x04];       // +0x34, not yet recovered.
    float mStartTime;                      /*!< The time the beam started. +0x38 */
    unsigned char mReserved3C[0x04];       // +0x3c, not yet recovered.
    Ship *mOwner;                          /*!< The ship that fired the beam. +0x40 */
    std::list<Ship::BeamData> *mOwnerList; /*!< The list of the ship that records it. +0x44 */
    unsigned char mReserved48[0x18];       // +0x48, not yet recovered.
    Beam *mNextFree;                       /*!< The next beam of the free list of the pool. +0x60 */
    unsigned char mReserved64[0x0c];       // +0x64, not yet recovered.
};
