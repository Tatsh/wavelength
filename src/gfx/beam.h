#pragma once

#include <list>

#include "gfx/beamparams.h"
#include "gfx/ship.h"
#include "math/color.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/line.h"
#include "rnd/mat.h"
#include "rnd/transformable.h"

/**
 * Beam a ship fires at a gem, a line that two waves ripple along.
 *
 * The RTTI names the class through the beam pools. The object is 0x70 bytes.
 */
class Beam {
public:
    /**
     * Construct an idle beam with a line and a material of its own.
     *
     * @ghidraAddress NTSC-U/C: 0x001ed2d0
     * @ghidraAddress PAL: 0x001f6070
     */
    Beam();

    /**
     * Destroy the line and the material of the beam.
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
     * Fire the beam for a ship, with waves of random frequencies and periods.
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
     * Stop the beam and hide it.
     *
     * @ghidraAddress NTSC-U/C: 0x001ed5c0
     * @ghidraAddress PAL: 0x001f6360
     */
    void Stop();

    /**
     * Ripple the line of the beam between its ends and colour it.
     *
     * @param direction The direction the waves lift the line in.
     * @param side The direction the second wave moves the line in.
     * @param color The colour an end of no alpha takes.
     * @return Whether the beam has ended, in which case nothing is changed.
     * @ghidraAddress NTSC-U/C: 0x001ed608
     * @ghidraAddress PAL: 0x001f63a8
     */
    bool Update(const Vector3 &direction, const Vector3 &side, const Color &color);

    /**
     * Move one end of the beam.
     *
     * @param nEnd The end, one of BeamParams::End.
     * @param pPosition The position, four floats.
     * @ghidraAddress NTSC-U/C: 0x001ed9c0
     * @ghidraAddress PAL: 0x001f6760
     */
    void SetEnd(int nEnd, const float *pPosition);

    const BeamParams *mParams;              /*!< The look, or null before Configure(). */
    unsigned char mReserved04[0x0c];        // +0x04, not yet recovered.
    float mEnds[2][Rnd::kXfmRowFloatCount]; /*!< The positions of the ends. */
    Rnd::Line *mLine;                       /*!< The line of the beam. */
    Rnd::Mat *mMat;                         /*!< The material of the line. */
    float mStartTime;                       /*!< The time the beam started. */
    float mEndTime;                         /*!< The time the beam ends. */
    Ship *mOwner;                           /*!< The ship that fired the beam. */
    std::list<Ship::BeamData> *mOwnerList;  /*!< The list of the ship that records it. */
    Vector2 mFrequencies;                   /*!< The waves along the beam of each wave. */
    Vector2 mTravelPeriods;                 /*!< The period of the travel of each wave. */
    unsigned char mReserved58[0x08];        // +0x58, not yet recovered.
    Beam *mNextFree;                        /*!< The next beam of the free list. */
    unsigned char mReserved64[0x0c];        // +0x64, not yet recovered.
};
