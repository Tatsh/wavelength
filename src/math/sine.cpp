#include "math/sine.h"

#include <math.h>

#include "os/system.h"
#include "script/dataarray.h"

namespace {

// Pi and its multiples as the image stores them.
constexpr float kPi = 3.14159274F;
constexpr float kHalfPi = 1.57079637F;
constexpr float kTwoOverPi = 0.636619746F;

// The full-turn table.
constexpr int kFullTableSize = 256;
constexpr int kFullTableMask = kFullTableSize - 1;
constexpr float kFullTableStep = 0.0245436933F; // Two pi over the table size.
constexpr float kFullTableScale = 40.7436638F;  // The table size over two pi.
constexpr float kFullTableRounding = 0.499989986F;

// Quadrants of a turn.
enum Quadrant { kQuadrantRising = 0, kQuadrantFalling = 1, kQuadrantNegative = 2 };
constexpr int kQuadrantMask = 3;

// The quarter-wave entries.
// NTSC-U/C: 0x003b2350
int gSinTableSize = 32;

// NTSC-U/C: 0x003b2354
float *gSinTable;

// Each quarter-wave entry less the one before it.
// NTSC-U/C: 0x003b2358
float *gSinSlopes;

// The quarter-wave entries per radian.
// NTSC-U/C: 0x002939f8, PAL: 0x0029d3c0 (static initialiser)
// NTSC-U/C: 0x00293a38, PAL: 0x0029d400 (constructor call)
// NTSC-U/C: 0x00512a88
float gSinTableScale =
    (static_cast<float>(gSinTableSize) + static_cast<float>(gSinTableSize)) / kPi;

// NTSC-U/C: 0x00512a90
float gFullSinTable[kFullTableSize];

// Each full-turn entry less the one before it.
// NTSC-U/C: 0x00512e90
float gFullSinSlopes[kFullTableSize];

// Interpolate the quarter-wave table at a fractional entry.
// NTSC-U/C: 0x00293818, PAL: 0x0029d1e0
float Lookup(float fEntry) {
    const int nEntry = static_cast<int>(fEntry);
    return gSinTable[nEntry] + ((fEntry - static_cast<float>(nEntry)) * gSinSlopes[nEntry]);
}

} // namespace

void SinTableInit() {
    SystemConfig()->FindArray("math", true)->FindInt("sin_table_size", &gSinTableSize, true);
    gSinTableScale = (static_cast<float>(gSinTableSize) + static_cast<float>(gSinTableSize)) / kPi;
    gSinTable = new float[gSinTableSize + 1];
    gSinSlopes = new float[gSinTableSize + 1];
    const float fStep = kHalfPi / static_cast<float>(gSinTableSize);
    for (int i = 0; i < gSinTableSize; ++i) {
        const float fSin = sinf(static_cast<float>(i) * fStep);
        gSinTable[i] = fSin;
        if (i != 0) {
            gSinSlopes[i - 1] = fSin - gSinTable[i - 1];
        }
    }
    gSinSlopes[gSinTableSize - 1] = 0.0F;
    gSinTable[gSinTableSize] = 1.0F;
    gSinSlopes[gSinTableSize] = 0.0F;

    for (int i = 0; i < kFullTableSize; ++i) {
        const float fSin = sinf(static_cast<float>(i) * kFullTableStep);
        gFullSinTable[i] = fSin;
        if (i != 0) {
            gFullSinSlopes[i - 1] = fSin - gFullSinTable[i - 1];
        }
    }
    gFullSinSlopes[kFullTableSize - 1] = 0.0F;
}

void SinTableTerminate() {
    delete[] gSinSlopes;
    delete[] gSinTable;
    gSinTable = nullptr;
    gSinSlopes = nullptr;
}

float SinApprox(float fRadians) {
    int nQuadrant = static_cast<int>(fRadians * kTwoOverPi);
    float fAngle = fRadians - (static_cast<float>(nQuadrant) * kHalfPi);
    if (fAngle < 0.0F) {
        fAngle += kHalfPi;
        --nQuadrant;
    }
    const float fEntry = fAngle * gSinTableScale;
    switch (nQuadrant & kQuadrantMask) {
    case kQuadrantRising:
        return Lookup(fEntry);
    case kQuadrantFalling:
        return Lookup(static_cast<float>(gSinTableSize) - fEntry);
    case kQuadrantNegative:
        return -Lookup(fEntry);
    default:
        return -Lookup(static_cast<float>(gSinTableSize) - fEntry);
    }
}

float FastSin(float fRadians) {
    if (fRadians < 0.0F) {
        const int nEntry = static_cast<int>((fRadians * -kFullTableScale) + kFullTableRounding);
        return -gFullSinTable[nEntry & kFullTableMask];
    }
    const int nEntry = static_cast<int>((fRadians * kFullTableScale) + kFullTableRounding);
    return gFullSinTable[nEntry & kFullTableMask];
}
