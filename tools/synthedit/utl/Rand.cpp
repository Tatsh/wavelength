#include "utl/Rand.h"

namespace {

// The linear congruential sequence that fills the table.
const unsigned int kSeedMultiplier = 0x41c64e6d;
const unsigned int kSeedIncrement = 0x3039;

// Each word takes the high halves of two consecutive values of the sequence.
const unsigned int kHighHalfMask = 0x7fff0000;
const int kHalfShift = 16;

// The second read position starts this far after the first.
const int kSecondIndex = 103;

// The seed of the shared generator before SeedRand() replaces it.
const int kInitialSeed = 666;

} // namespace

// 0x100c45f8
static Rand gRand(kInitialSeed);

Rand::Rand(int seed) : mIndex1(0), mIndex2(0) {
    Seed(seed);
}

void Rand::Seed(int seed) {
    unsigned int value = seed;
    for (int i = 0; i < kTableSize; ++i) {
        value = value * kSeedMultiplier + kSeedIncrement;
        const unsigned int low = value >> kHalfShift;
        value = value * kSeedMultiplier + kSeedIncrement;
        mTable[i] = (value & kHighHalfMask) | low;
    }
    mIndex1 = 0;
    mIndex2 = kSecondIndex;
}

void SeedRand(int seed) {
    gRand.Seed(seed);
}
