#include "synth_s/random.h"

namespace {

constexpr unsigned int kMultiplier = 1103515245;
constexpr unsigned int kIncrement = 12345;
constexpr unsigned int kHighBitsMask = 0x7fff0000;

} // namespace

// NTSC-U/C: 0x00015520
unsigned int Random::sSeed;

void Random::Seed(unsigned int seed) {
    sSeed = seed;
}

int Random::Range(int low, int high) {
    unsigned int state = (sSeed * kMultiplier) + kIncrement;
    const unsigned int lowBits = state >> 16;
    state = (state * kMultiplier) + kIncrement;
    sSeed = state;
    const int value = static_cast<int>(lowBits | (state & kHighBitsMask));
    return low + (value % (high - low));
}
