#include "math/rand.h"

namespace {

// The linear congruential step Seed() fills the table with.
constexpr unsigned int kSeedMultiplier = 0x41c64e6d;
constexpr unsigned int kSeedIncrement = 12345;
constexpr unsigned int kHighHalfShift = 16;
constexpr unsigned int kHighBitsMask = 0x7fff0000;

// Both indices wrap to zero here.
constexpr int kWrap = 249;

// The lag index after seeding.
constexpr int kLag = 103;

// Float() keeps the low sixteen bits of a draw and scales them into [0, 1).
constexpr int kFloatBitsMask = 0xffff;
constexpr float kFloatScale = 1.0F / 65536.0F;

// The seed of the shared generator.
constexpr int kSharedSeed = 666;

// NTSC-U/C: 0x00292490, PAL: 0x0029be58 (static initialiser)
// NTSC-U/C: 0x002924c0, PAL: 0x0029be88 (constructor call)
// NTSC-U/C: 0x00512680
Rand gRand(kSharedSeed);

} // namespace

Rand::Rand(int nSeed) : mIndex(0), mLagIndex(0) {
    Seed(nSeed);
}

void Rand::Seed(int nSeed) {
    unsigned int nValue = static_cast<unsigned int>(nSeed);
    for (int i = 0; i < kTableSize; ++i) {
        nValue = (nValue * kSeedMultiplier) + kSeedIncrement;
        const unsigned int nHigh = nValue >> kHighHalfShift;
        nValue = (nValue * kSeedMultiplier) + kSeedIncrement;
        mTable[i] = static_cast<int>(nHigh | (nValue & kHighBitsMask));
    }
    mIndex = 0;
    mLagIndex = kLag;
}

int Rand::Int(int nLow, int nHigh) {
    return nLow + (Int() % (nHigh - nLow));
}

float Rand::Float(float fLow, float fHigh) {
    return fLow + (Float() * (fHigh - fLow));
}

float Rand::Float() {
    return static_cast<float>(Int() & kFloatBitsMask) * kFloatScale;
}

int Rand::Int() {
    const int nValue = mTable[mIndex] ^ mTable[mLagIndex];
    mTable[mIndex] = nValue;
    if (++mIndex >= kWrap) {
        mIndex = 0;
    }
    if (++mLagIndex >= kWrap) {
        mLagIndex = 0;
    }
    return nValue;
}

void SeedRand(int nSeed) {
    gRand.Seed(nSeed);
}

int RandomWord() {
    return gRand.Int();
}

int RandomInt(int nLow, int nHigh) {
    return gRand.Int(nLow, nHigh);
}

float RandomFraction() {
    return gRand.Float();
}

float RandomFloat(float fLow, float fHigh) {
    return gRand.Float(fLow, fHigh);
}
