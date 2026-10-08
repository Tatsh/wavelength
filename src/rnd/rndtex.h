#pragma once

#include "rnd/rndobject.h"

/**
 * Texture.
 *
 * The RTTI includes the class name and records RndObject as the one base. Only the members ported
 * so far are declared.
 */
class RndTex : public RndObject {
public:
    int mWidth;  /*!< Width in pixels. */
    int mHeight; /*!< Height in pixels. */
};
