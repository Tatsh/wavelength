#pragma once

#include "math/color.h"

/**
 * The model of one part of an avatar that an AvatarPlayer draws.
 *
 * The name is inferred. Only the member the Freq maker reaches is declared.
 */
class AvatarPart {
public:
    /**
     * Colour the model.
     *
     * @param pColor The colour.
     * @ghidraAddress NTSC-U/C: 0x00274448
     * @ghidraAddress PAL: 0x0027df68
     */
    void SetColor(const Color *pColor);
};
