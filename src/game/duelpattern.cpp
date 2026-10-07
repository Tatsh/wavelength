#include "game/duelpattern.h"

namespace {

constexpr char kAllowedStep = 'x';
constexpr int kDefaultAllowedSteps = 29;

} // namespace

DuelPattern::DuelPattern(const DataArray *pLanes) {
    if (pLanes != nullptr) {
        for (int nLane = 0; nLane < kDuelPatternLanes; ++nLane) {
            const char *pszSteps = pLanes->Sym(nLane);
            for (int nStep = 0; nStep < kDuelPatternSteps; ++nStep) {
                mAllowed[nStep][nLane] = pszSteps[nStep] == kAllowedStep;
            }
        }
    } else {
        for (int nLane = 0; nLane < kDuelPatternLanes; ++nLane) {
            for (int nStep = 0; nStep < kDuelPatternSteps; ++nStep) {
                mAllowed[nStep][nLane] = nStep < kDefaultAllowedSteps;
            }
        }
    }
}
