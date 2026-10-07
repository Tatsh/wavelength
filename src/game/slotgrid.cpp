#include "game/slotgrid.h"

namespace {

constexpr int kNoPattern = -1;

} // namespace

SlotGrid::SlotGrid(int nSections) : mPatterns(nSections, kNoPattern), mNumFree(nSections) {
}

void SlotGrid::SetPattern(int nPattern, int nSection) {
    mPatterns[nSection] = nPattern;
    int nFree = 0;
    for (auto pattern : mPatterns) {
        if (pattern == kNoPattern) {
            ++nFree;
        }
    }
    mNumFree = nFree;
}

bool SlotGrid::HasPattern(int nSection) const {
    return mPatterns[nSection] != kNoPattern;
}

int SlotGrid::GetPattern(int nSection) const {
    return mPatterns[nSection];
}

int SlotGrid::GetNumSections() const {
    return static_cast<int>(mPatterns.size());
}

int SlotGrid::GetNumFree() const {
    return mNumFree;
}
