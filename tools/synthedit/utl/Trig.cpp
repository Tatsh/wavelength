#include "utl/Trig.h"

#include <math.h>

#include "os/System.h"
#include "utl/Data.h"

namespace {

const float kTwoOverPi = 0.63661975f;
const float kHalfPi = 1.5707964f;

// The fixed table covers a full turn in this many steps.
const int kSin256Size = 256;
const float kSin256Step = 0.024543693f;

} // namespace

// The entries of the quarter-turn table. The configuration replaces this default.
// 0x1003aad4
static int gSinTableSize = 32;

// Table entries per radian.
// 0x100c45ec
static float gSinTableScale;

// The sine of each step of a quarter turn, and 1 after the last.
// 0x100c45f0
static float *gSinTable;

// The difference between each entry of gSinTable and the next.
// 0x100c45f4
static float *gSinDeltaTable;

// The sine of each step of a full turn.
// 0x100c41ec
static float gSin256[kSin256Size];

// The difference between each entry of gSin256 and the next.
// 0x100c3dec
static float gSin256Deltas[kSin256Size];

void TrigInit() {
    SystemConfig()->FindArray("math", true)->FindInt("sin_table_size", &gSinTableSize, true);
    gSinTableScale = static_cast<float>(gSinTableSize) * kTwoOverPi;
    gSinTable = new float[gSinTableSize + 1];
    gSinDeltaTable = new float[gSinTableSize + 1];
    const float step = kHalfPi / static_cast<float>(gSinTableSize);
    int i;
    for (i = 0; i < gSinTableSize; ++i) {
        const float s = static_cast<float>(sin(static_cast<float>(i) * step));
        gSinTable[i] = s;
        if (i != 0) {
            gSinDeltaTable[i - 1] = s - gSinTable[i - 1];
        }
    }
    gSinDeltaTable[gSinTableSize - 1] = 0.0f;
    gSinTable[gSinTableSize] = 1.0f;
    gSinDeltaTable[gSinTableSize] = 0.0f;
    for (i = 0; i < kSin256Size; ++i) {
        const float s = static_cast<float>(sin(static_cast<float>(i) * kSin256Step));
        gSin256[i] = s;
        if (i != 0) {
            gSin256Deltas[i - 1] = s - gSin256[i - 1];
        }
    }
    gSin256Deltas[kSin256Size - 1] = 0.0f;
}

void TrigTerminate() {
    delete[] gSinDeltaTable;
    delete[] gSinTable;
    gSinTable = NULL;
    gSinDeltaTable = NULL;
}
