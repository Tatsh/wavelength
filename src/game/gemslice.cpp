#include "game/gemslice.h"

GemSlice::GemSlice() : mGems(nullptr), mOffset(0) {
}

GemSlice::GemSlice(const std::vector<PitchGem> *pGems, int nOffset)
    : mGems(pGems), mOffset(nOffset) {
}

GemSlice::~GemSlice() {
}

GemSlice &GemSlice::operator=(const GemSlice &other) {
    if (this != &other) {
        mGems = other.mGems;
        mOffset = other.mOffset;
    }
    return *this;
}
