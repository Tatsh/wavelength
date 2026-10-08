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
     * Report the ratio of the height of the display to its width, fixed at 3 to 4.
     *
     * @return 0.75.
     * @ghidraAddress NTSC-U/C: 0x0037c858
     * @ghidraAddress PAL: 0x003eaf88
     */
    float AspectRatio() override {
        return kAspect;
    }

    /** The ratio AspectRatio() reports. */
    static constexpr float kAspect = 0.75f;

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

    /**
     * Whether the full-screen blur runs. The blur action and the enlarging blur set it, and the
     * head-up display clears it when it goes away. +0x46c
     */
    int mBlurActive;

    /**
     * Draw the frame back over itself, offset and translucent, inside mBlurRect.
     *
     * The title is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00211930
     * @ghidraAddress PAL: 0x0021a748
     */
    void DrawBlur();

    float mBlurRect[4]; /*!< The part of the screen DrawBlur() covers, x, y, w, and h. +0x470 */
    float mBlurAmount;  /*!< The opacity of the frame DrawBlur() draws. +0x480 */
    int mBlurCount;     /*!< The offset of the frame DrawBlur() draws, in pixels. +0x484 */
};

/**
 * The renderer.
 *
 * @ghidraAddress NTSC-U/C: 0x0043ba00
 */
extern Ps ThePs;
