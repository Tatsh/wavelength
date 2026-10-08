#pragma once

#include "rnd/particlesys.h"
#include "rnd/transformable.h"

/**
 * Particle system that hangs from a transform and is drawn as a reflection of it.
 *
 * The class emits no RTTI, and the name is inferred. The object is 0xc bytes. Init() fills it,
 * and no constructor runs first.
 */
class ParticleArm {
public:
    /**
     * Destroy the arm. The body does nothing beyond the release of a heap object.
     *
     * @ghidraAddress NTSC-U/C: 0x001e3388
     * @ghidraAddress PAL: 0x001ec128
     */
    ~ParticleArm();

    /**
     * Attach a particle system to a transform and scale it.
     *
     * @param pSys The particle system.
     * @param pTrans The transform the system hangs from.
     * @param fScale The scale.
     * @ghidraAddress NTSC-U/C: 0x001e3310
     * @ghidraAddress PAL: 0x001ec0b0
     */
    void Init(Rnd::ParticleSys *pSys, Rnd::Transformable *pTrans, float fScale);

    /**
     * Record the reciprocal of a scale and scale the particle system by it.
     *
     * @param fScale The scale.
     * @ghidraAddress NTSC-U/C: 0x001e33b0
     * @ghidraAddress PAL: 0x001ec150
     */
    void SetScale(float fScale);

    /**
     * Scale the size, the speed, the emission box, and the transform of a particle system.
     *
     * @param pSys The particle system.
     * @param fScale The scale.
     * @ghidraAddress NTSC-U/C: 0x001e33e8
     * @ghidraAddress PAL: 0x001ec188
     */
    void Scale(Rnd::ParticleSys *pSys, float fScale);

    /**
     * Draw the particle system through the current camera.
     *
     * @ghidraAddress NTSC-U/C: 0x001e34d8
     * @ghidraAddress PAL: 0x001ec278
     */
    void Draw();

    Rnd::ParticleSys *mSys;     /*!< The particle system. */
    Rnd::Transformable *mTrans; /*!< The transform the system hangs from. */
    float mInvScale;            /*!< The reciprocal of the scale. */
};
