#pragma once

#include "rnd/particlesys.h"

/**
 * The sprite gems of one type, drawn as the particles of a particle system.
 *
 * The class is not polymorphic, and the name is inferred. The object is 4 bytes.
 */
class SpriteGroup {
public:
    /**
     * Find the particle system and size its pool.
     *
     * @param pszName The name of the particle system, without its `.ps` extension.
     * @param nMaxGems The most gems the tunnel places at once.
     * @ghidraAddress NTSC-U/C: 0x001f4480
     * @ghidraAddress PAL: 0x001fd220
     */
    SpriteGroup(const char *pszName, int nMaxGems);

    Rnd::ParticleSys *mParticles; /*!< The particle system. */
};
