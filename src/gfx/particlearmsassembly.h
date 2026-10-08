#pragma once

#include "gfx/tnlgeom.h"
#include "math/interpolator.h"
#include "rnd/particlesys.h"
#include "rnd/transanim.h"
#include "script/dataarray.h"

/**
 * A particle system that sprays from arms turning about a point ahead on the tunnel.
 *
 * The class is not polymorphic. The RTTI of a pointer to it records the name. The object is 0x20
 * bytes. Each arm spawns its share of the particles of a frame from its own transform.
 */
class ParticleArmsAssembly {
public:
    /**
     * Create the particle system and load the arms.
     *
     * @param pData The entry, the name of the particle system followed by its settings.
     * @ghidraAddress NTSC-U/C: 0x001f6248
     * @ghidraAddress PAL: 0x001fefe8
     */
    explicit ParticleArmsAssembly(DataArray *pData);

    /**
     * Delete the particle system and the curve.
     *
     * @ghidraAddress NTSC-U/C: 0x001f6bc8
     * @ghidraAddress PAL: 0x001ff968
     */
    ~ParticleArmsAssembly();

    /**
     * Load the arms and the particle settings, and stop the emission.
     *
     * @param pData The entry.
     * @ghidraAddress NTSC-U/C: 0x001f63b8
     * @ghidraAddress PAL: 0x001ff158
     */
    void LoadConfig(DataArray *pData);

    /**
     * Turn the arms and emit their particles up to a tick.
     *
     * A system with no live particles and no emission only records the tick.
     *
     * @param pGeom The geometry of the tunnel, which places the arms when there is no path.
     * @param flTick The tick of the song.
     * @ghidraAddress NTSC-U/C: 0x001f6c50
     * @ghidraAddress PAL: 0x001ff9f0
     */
    void Poll(TnlGeom *pGeom, float flTick);

    /**
     * Move the point the arms turn about over a span of ticks.
     *
     * @param flFromTicks The ticks ahead at the start of the move.
     * @param flToTicks The ticks ahead at the end of the move.
     * @param flTicks The length of the move.
     * @ghidraAddress NTSC-U/C: 0x001f6f08
     * @ghidraAddress PAL: 0x001ffca8
     */
    void Move(float flFromTicks, float flToTicks, float flTicks);

    /**
     * Report the use of the particles through the debug output.
     *
     * @ghidraAddress NTSC-U/C: 0x001f6f40
     * @ghidraAddress PAL: 0x001ffce0
     */
    void Report();

    /**
     * Report the name of the particle system.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x001f6fc0
     * @ghidraAddress PAL: 0x001ffd60
     */
    const char *Name() const;

    Rnd::ParticleSys *mSys;    /*!< The particle system. */
    Rnd::TransAnim *mPath;     /*!< The path the arms follow, `path`, or null. */
    Interpolator *mTicksAhead; /*!< The ticks ahead of the song the arms turn at, over ticks. */
    int mNumArms;              /*!< The number of arms, `num_arms`. */
    float mRotationSpeed;      /*!< The turn of the arms per tick, in radians. */
    float mArmLength;          /*!< The length of an arm, `arm_length`. */
    float mTick;               /*!< The tick the particles are emitted up to. */
    float mAngle;              /*!< The angle of the first arm, in radians. */
};
