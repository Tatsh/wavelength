#pragma once

#include "rnd/transformable.h"

/**
 * Camera of the players in the tunnel, with its slides, its zoom, and its shakes.
 *
 * The RTTI names the class. Only the members the ships use are declared so far.
 */
class PlayerCamFX {
public:
    /**
     * Shake the camera.
     *
     * @param fAmount The size of the shake.
     * @ghidraAddress NTSC-U/C: 0x001f9280
     * @ghidraAddress PAL: 0x00202020
     */
    void Kick(float fAmount);

    unsigned char mReserved00[0x20]; // +0x00, not yet recovered.
    /*!< The transform of the camera. +0x20 */
    float mXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    /*!< The transform of the camera along its slide. +0x60 */
    float mSlideXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    unsigned char mReservedA0[0x3c]; // +0xa0, not yet recovered.
    float mZoomOut; /*!< Non-zero while the camera is pulled back from the ship. +0xdc */
};
