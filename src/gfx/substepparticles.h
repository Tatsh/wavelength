#pragma once

#include "math/vector3.h"
#include "rnd/particlesys.h"

/**
 * Particle system that emits in several steps along the path its transform moved since the last
 * update, so that a fast move leaves an even trail.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x28 bytes.
 */
class SubstepParticles {
public:
    /**
     * Construct the steps of a particle system.
     *
     * @param nSteps The number of steps of an update.
     * @param pSys The particle system.
     * @ghidraAddress NTSC-U/C: 0x001e3978
     * @ghidraAddress PAL: 0x001ec718
     */
    SubstepParticles(int nSteps, Rnd::ParticleSys *pSys);

    /**
     * Forget the last update, so the next one only records the position.
     *
     * @ghidraAddress NTSC-U/C: 0x001e39b8
     * @ghidraAddress PAL: 0x001ec758
     */
    void Reset();

    /**
     * Set the number of steps of an update.
     *
     * @param nSteps The number of steps.
     * @ghidraAddress NTSC-U/C: 0x001e39d0
     * @ghidraAddress PAL: 0x001ec770
     */
    void SetSteps(int nSteps);

    /**
     * Pose the particle system and emit its particles in steps since the last update.
     *
     * @param fFrame The frame of the particle system.
     * @param fTime The time of the update.
     * @ghidraAddress NTSC-U/C: 0x001e39f8
     * @ghidraAddress PAL: 0x001ec798
     */
    void Update(float fFrame, float fTime);

    Rnd::ParticleSys *mSys; /*!< The particle system. */
    float mLastTime;        /*!< The time of the last update, or -1e9 for none. */
    Vector3 mLastPos;       /*!< The world position of the system at the last update. */
    int mSteps;             /*!< The number of steps of an update. */
    float mInvSteps;        /*!< One over mSteps. */
};
