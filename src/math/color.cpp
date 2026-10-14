#include "math/color.h"

namespace {

// The ceiling the vector unit broadcasts into the minimum stage.
constexpr float kColorCeiling = 1.0f;

// The floor the vector unit broadcasts into the maximum stage.
constexpr float kColorFloor = 0.0f;

inline float ClampComponent(float flValue) {
    if (flValue < kColorFloor) {
        return kColorFloor;
    }
    if (flValue > kColorCeiling) {
        return kColorCeiling;
    }
    return flValue;
}

} // namespace

PrnStream &operator<<(PrnStream &stream, const Color &color) {
    stream << "(r:" << color.r << " g:" << color.g << " b:" << color.b << " a:" << color.a << ")";
    return stream;
}
