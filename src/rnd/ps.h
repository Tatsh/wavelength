#pragma once

#include "rnd/rndrenderer.h"

/**
 * The PlayStation 2 renderer.
 *
 * The RTTI includes the name and records RndRenderer as the one base. ThePs is the one instance,
 * and TheRnd points at it. Only the members the metagame uses are declared.
 */
class Ps : public RndRenderer {
public:
    /**
     * Give the display up to the movie player.
     *
     * The scratchpad is saved, the GS path is drained, and the GIF interrupt handler is removed.
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00210b88
     * @ghidraAddress PAL: 0x002199a0
     */
    void ReleaseForMovie();

    /**
     * Take the display back from the movie player and set the display mode up again.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00210bb8
     * @ghidraAddress PAL: 0x002199d0
     */
    void RestoreAfterMovie();
};

/**
 * The renderer.
 *
 * @ghidraAddress NTSC-U/C: 0x0043ba00
 */
extern Ps ThePs;
