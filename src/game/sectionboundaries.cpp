#include "game/sectionboundaries.h"

#include <algorithm>

namespace {

// The measure at which the first section starts.
constexpr int kFirstSectionStart = 0;

} // namespace

int SectionBoundaries::NumSections() const {
    return static_cast<int>(mEnds.size());
}

bool SectionBoundaries::IsPastEnd(int nMeasure) const {
    return nMeasure >= mEnds.back();
}

int SectionBoundaries::SectionStart(int nSection) const {
    if (nSection == 0) {
        return kFirstSectionStart;
    }
    return mEnds[nSection - 1];
}

int SectionBoundaries::SectionEnd(int nSection) const {
    return mEnds[nSection];
}

void SectionBoundaries::GetSectionRange(int nSection, int *pStart, int *pEnd) const {
    if (nSection == 0) {
        *pStart = kFirstSectionStart;
    } else {
        *pStart = mEnds[nSection - 1];
    }
    *pEnd = mEnds[nSection];
}

int SectionBoundaries::SectionAt(int nMeasure) const {
    (void)IsPastEnd(nMeasure); // Yes, the binary discards this call's result.
    return static_cast<int>(std::upper_bound(mEnds.begin(), mEnds.end(), nMeasure) - mEnds.begin());
}
