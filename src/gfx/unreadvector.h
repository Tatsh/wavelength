#pragma once

#include "math/vector3.h"

namespace {

/**
 * A vector of 7 in each component that each including unit sets as its first static and never
 * reads.
 *
 * The header is inferred. Each of the 16 units that include it has its own copy, among them
 * NTSC-U/C 0x0043ae90 (GfxManager), 0x0043b100 (Overlay), 0x0043b1f0 (Ship), 0x0043b310
 * (GfxTunnel), and 0x0043b580 (GfxArena).
 */
[[maybe_unused]] Vector3 sUnreadVector{7.0f, 7.0f, 7.0f, 7.0f};

} // namespace
