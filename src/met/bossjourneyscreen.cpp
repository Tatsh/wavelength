#include "met/bossjourneyscreen.h"

#include "gfx/gfxmanager.h"
#include "met/metagame.h"

BossJourneyScreen::BossJourneyScreen(DataArray *pData) : FreqScreen(pData), mDone(0) {
}

void BossJourneyScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mDone = 0;
    FreqScreen::Enter(pPrevScreen, fTime);
}

void BossJourneyScreen::Poll(float fTime) {
    if (!mDone && TheGfxManager.IsBossJourneyDone()) {
        TheMetagame.AdvanceUnlocks();
        mDone = 1;
    }
    UIScreen::Poll(fTime);
}
