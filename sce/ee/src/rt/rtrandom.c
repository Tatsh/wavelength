#include "rt/rtrandom.h"

#include <eeregs.h>

#include "rt/rtclock.h"

enum {
    kRandomStateSize = 624,
    kRandomShift = 397,
    kRandomDefaultSeed = 4357,
    kRandomSeedMultiplier = 69069,
    kRandomMaxSeedCount = 623,
    // The words of the entropy RtSeedRandomFromEntropy() gathers on the stack.
    kEntropyWordCount = 64,
};

#define RANDOM_MATRIX 0x9908b0dfU
#define RANDOM_UPPER_MASK 0x80000000U
#define RANDOM_LOWER_MASK 0x7fffffffU
#define RANDOM_TEMPER_MASK_B 0x9d2c5680U
#define RANDOM_TEMPER_MASK_C 0xefc60000U

// The state has one word more than the generator uses. RtSeedRandomFromArray() writes it.
// NTSC-U/C: 0x003b61b0, PAL: 0x00435070
static unsigned int g_auRandomState[kRandomStateSize + 1];
// An index past the state makes the first draw seed the generator.
// NTSC-U/C: 0x003b6b74
static int g_nRandomIndex = kRandomStateSize;

// The words RtSeedRandomFromEntropy() gathers.
typedef union {
    struct {
        unsigned int uSeed;
        RtTime uptime;
        char szWallClock[12];
        unsigned int auTimerCounts[4];
    } parts;
    // Yes, the binary seeds with the words after the parts uninitialised.
    unsigned int auWords[kEntropyWordCount];
} RtEntropy;

void RtSeedRandom(unsigned int uSeed) {
    int i;

    g_nRandomIndex = kRandomStateSize - 1;
    uSeed |= 1;
    g_auRandomState[0] = uSeed;
    for (i = 1; i < kRandomStateSize; ++i) {
        uSeed *= kRandomSeedMultiplier;
        g_auRandomState[i] = uSeed;
    }
}

void RtSeedRandomFromArray(const unsigned int *puKey, unsigned int uCount) {
    unsigned int uValue;
    int i;

    if (uCount >= kRandomStateSize) {
        uCount = kRandomMaxSeedCount;
    }
    g_nRandomIndex = kRandomStateSize - 1;
    uValue = 0xffffffffU;
    g_auRandomState[0] = uValue;
    for (i = 1; i <= kRandomStateSize; ++i) {
        if (uCount != 0) {
            --uCount;
            uValue ^= *puKey++;
        }
        uValue *= kRandomSeedMultiplier;
        g_auRandomState[i] = uValue;
    }
}

void RtRegenerateRandom(void) {
    unsigned int uValue;
    int i;

    if (g_nRandomIndex > kRandomStateSize) {
        RtSeedRandom(kRandomDefaultSeed);
    }
    g_nRandomIndex = 0;
    for (i = 0; i < kRandomStateSize - kRandomShift; ++i) {
        uValue =
            (g_auRandomState[i] & RANDOM_UPPER_MASK) | (g_auRandomState[i + 1] & RANDOM_LOWER_MASK);
        g_auRandomState[i] = g_auRandomState[i + kRandomShift] ^ (uValue >> 1) ^
                             ((g_auRandomState[i + 1] & 1) != 0 ? RANDOM_MATRIX : 0);
    }
    for (; i < kRandomStateSize - 1; ++i) {
        uValue =
            (g_auRandomState[i] & RANDOM_UPPER_MASK) | (g_auRandomState[i + 1] & RANDOM_LOWER_MASK);
        g_auRandomState[i] = g_auRandomState[i + kRandomShift - kRandomStateSize] ^ (uValue >> 1) ^
                             ((g_auRandomState[i + 1] & 1) != 0 ? RANDOM_MATRIX : 0);
    }
    uValue = (g_auRandomState[kRandomStateSize - 1] & RANDOM_UPPER_MASK) |
             (g_auRandomState[0] & RANDOM_LOWER_MASK);
    g_auRandomState[kRandomStateSize - 1] = g_auRandomState[kRandomShift - 1] ^ (uValue >> 1) ^
                                            ((g_auRandomState[0] & 1) != 0 ? RANDOM_MATRIX : 0);
}

unsigned int RtRandom(void) {
    unsigned int uValue;

    ++g_nRandomIndex;
    if (g_nRandomIndex >= kRandomStateSize) {
        RtRegenerateRandom();
    }
    uValue = g_auRandomState[g_nRandomIndex];
    uValue ^= uValue >> 11;
    uValue ^= (uValue << 7) & RANDOM_TEMPER_MASK_B;
    uValue ^= (uValue << 15) & RANDOM_TEMPER_MASK_C;
    uValue ^= uValue >> 18;
    return uValue;
}

// NTSC-U/C: 0x002aa9b0, PAL: 0x002b4920
static void RtSeedRandomFromClocks(int nSeed) {
    RtEntropy entropy;

    entropy.parts.uSeed = nSeed;
    (void)RtGetUptime(&entropy.parts.uptime);           // Yes, the binary discards the result.
    (void)RtFormatWallClock(entropy.parts.szWallClock); // Yes, the binary discards the result.
    entropy.parts.auTimerCounts[0] = *T0_COUNT;
    entropy.parts.auTimerCounts[1] = *T1_COUNT;
    entropy.parts.auTimerCounts[2] = *T2_COUNT;
    entropy.parts.auTimerCounts[3] = *T3_COUNT;
    RtSeedRandomFromArray(entropy.auWords, kEntropyWordCount);
}

void RtSeedRandomFromEntropy(unsigned int uSeed) {
    RtSeedRandomFromClocks(uSeed);
}
