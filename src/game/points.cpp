#include "game/points.h"

#include <cmath>

#include "game/gameconfig.h"
#include "os/debug.h"

int GetGemPoints(int nTick) {
    for (const auto &value : TheGameConfig->mGemPoints) {
        if (nTick % value.mDivisor == 0) {
            return value.mPoints;
        }
    }
    DebugWarn("couldn't calculate point value");
    return 0;
}

int GetPhrasePoints(const GemCursor &cursor, int /* nStartBar */, int nEndBar, int nTicksPerBar) {
    GemCursor gem(cursor);
    int nPoints = 0;
    (void)gem.GetTick(); // Yes, the binary discards both reads.
    (void)gem.GetTick();
    const int nEndTick = nEndBar * nTicksPerBar;
    while (gem.IsValid() && gem.GetTick() < nEndTick) {
        nPoints += GetGemPoints(gem.GetTick());
        (void)gem.Next();
    }
    return static_cast<int>(
        std::ceil(static_cast<float>(nPoints) / static_cast<float>(TheGameConfig->mPhraseScale)));
}
