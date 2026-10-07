#include "met/partunlockdonescreen.h"

#include "met/metagame.h"
#include "os/system.h"

namespace {

constexpr float kAdvanceDelayMs = 10000.0f;

// The value of mAdvanceMs while no unlock is due.
constexpr float kNever = 1.0e30f;

} // namespace

PartUnlockDoneScreen::PartUnlockDoneScreen(DataArray *pData)
    : UnlockScreen(pData), mAdvanceMs(kNever) {
}

void PartUnlockDoneScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mAdvanceMs = SystemMs() + kAdvanceDelayMs;
}

void PartUnlockDoneScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mAdvanceMs < SystemMs()) {
        mAdvanceMs = kNever;
        TheMetagame.AdvanceUnlocks();
    }
}
