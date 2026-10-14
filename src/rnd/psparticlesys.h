#pragma once

#include "rnd/rndparticlesys.h"

/**
 * Particle system, PlayStation 2.
 *
 * The RTTI includes the class name and records RndParticleSys as the one base. The class adds no
 * data member. Only sprite systems draw. VU1 expands each particle into a sprite of its size.
 */
class PsParticleSys : public RndParticleSys {
public:
    /**
     * Construct a system.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0021a378
     * @ghidraAddress PAL: 0x00223190
     */
    explicit PsParticleSys(const char *pszName);

    /**
     * @ghidraAddress NTSC-U/C: 0x0037d710
     * @ghidraAddress PAL: 0x003ebe40
     */
    ~PsParticleSys() override {
    }

    /**
     * Draw the live particles of a sprite system in each pass of the material, without writing
     * depth.
     *
     * The particles test depth only when mReadZ is set. A system of another mode draws nothing.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x0021a2a8
     * @ghidraAddress PAL: 0x002230c0
     */
    int DrawShowing() override;

    /**
     * Send the live particles to VU1 in batches of up to 162, each batch followed by a frame
     * flush.
     *
     * The first batch starts the sprite microprogram, and the others continue it. Each particle
     * sends its colour and its position, with half its size in the fourth word of the position.
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00219f28
     * @ghidraAddress PAL: 0x00222d40
     */
    void DrawParticles();

    /**
     * Create a system.
     *
     * @param pszName The registry key.
     * @return The system.
     * @ghidraAddress NTSC-U/C: 0x0037d8c8
     * @ghidraAddress PAL: 0x003ebff8
     */
    static RndObject *New(const char *pszName) {
        return new PsParticleSys(pszName);
    }
};
