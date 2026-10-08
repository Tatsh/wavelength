#pragma once

#include "rnd/transformable.h"

/**
 * Slide of the camera of the tunnel, `tnl cam slide1`.
 *
 * The class emits no RTTI, and its title is inferred. Only the members the display calls are
 * declared.
 */
class CamSlide {
public:
    /**
     * Find the transform of the slide.
     *
     * @ghidraAddress NTSC-U/C: 0x001e38f0
     * @ghidraAddress PAL: 0x001ec690
     */
    static void Init();

    /**
     * Forget the transform of the slide.
     *
     * @ghidraAddress NTSC-U/C: 0x001e3968
     * @ghidraAddress PAL: 0x001ec708
     */
    static void Terminate();

    /**
     * The transform of the slide, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa88
     */
    static Rnd::Transformable *sSlide;
};
