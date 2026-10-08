#pragma once

#include "math/vector3.h"
#include "rnd/particle.h"
#include "rnd/particlesys.h"

/**
 * A flash at a gem, one particle that shrinks away.
 *
 * The RTTI identifies the class, which is not polymorphic. The object is 0xc bytes. The tunnel
 * keeps a pool of them on the particle system `gem_flash.ps`.
 */
class TnlFlashFX {
public:
    /**
     * Construct an idle flash.
     *
     * @param pSys The particle system the flash draws with.
     * @ghidraAddress NTSC-U/C: 0x001ef6b8
     * @ghidraAddress PAL: 0x001f8458
     */
    explicit TnlFlashFX(Rnd::ParticleSys *pSys);

    /**
     * Return the particle of a running flash.
     *
     * @ghidraAddress NTSC-U/C: 0x001ef6d0
     * @ghidraAddress PAL: 0x001f8470
     */
    ~TnlFlashFX();

    /**
     * Start the flash if it is idle.
     *
     * @param pPos The position of the flash.
     * @param flSize The size the flash starts at.
     * @param flShrink The size the flash loses per unit of time.
     * @return The particle of the flash, or null for a flash that is already running.
     * @ghidraAddress NTSC-U/C: 0x001ef728
     * @ghidraAddress PAL: 0x001f84c8
     */
    Rnd::Particle *Start(const Vector3 *pPos, float flSize, float flShrink);

    /**
     * Stop the flash at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001ef7c8
     * @ghidraAddress PAL: 0x001f8568
     */
    void Stop();

    /**
     * Shrink the flash, and stop it once it has no size.
     *
     * @param flDelta The time since the last update.
     * @ghidraAddress NTSC-U/C: 0x001ef800
     * @ghidraAddress PAL: 0x001f85a0
     */
    void Poll(float flDelta);

    Rnd::ParticleSys *mSys;   /*!< The particle system the flash draws with. */
    Rnd::Particle *mParticle; /*!< The particle of a running flash, or null. */
    float mShrink;            /*!< The size the flash loses per unit of time. */
};
