#pragma once

#include "met/errorscreen.h"

/**
 * Memory card screen that loads the remix the game database describes.
 *
 * The RTTI records the class as deriving from ErrorScreen. The front-end description's
 * `load_remix` screen is one. Only the inherited routines its users here call are needed, and the
 * routines of the class are not reconstructed.
 */
class LoadRemixScreen : public ErrorScreen {};
