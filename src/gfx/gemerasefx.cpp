#include "gfx/gemerasefx.h"

#include <algorithm>

#include "game/gamedb.h"

GemEraseFX::Gem::Gem(float flTime, char nTrack, char nPlayer, float flTick, float flLateral)
    : mTime(flTime), mTrack(nTrack), mPlayer(nPlayer), mTick(flTick), mLateral(flLateral) {
}

GemEraseFX::GemEraseFX(GfxTunnel *pTunnel) : mTunnel(pTunnel) {
}

GemEraseFX::~GemEraseFX() {
}

void GemEraseFX::Add(char nTrack, char nPlayer, float flTime, float flTick, float flLateral) {
    const auto it = std::lower_bound(mGems.begin(), mGems.end(), flTime, Gem::Before);
    mGems.insert(it, Gem(flTime, nTrack, nPlayer, flTick, flLateral));
}

void GemEraseFX::Clear() {
    mGems.clear();
}

void GemEraseFX::Poll() {
    const float flTime = TheGameDb->mSongTime;
    auto it = mGems.begin();
    while (it != mGems.end()) {
        if (it->mTime <= flTime) {
            const std::vector<Rnd::View *> *pViews;
            if (it->mPlayer >= 0) {
                pViews = &mTunnel->mPlayerBurstViews[it->mPlayer];
            } else {
                pViews = &mTunnel->mGemBurstViews[0];
            }
            (void)mTunnel->Burst(pViews, it->mTrack, true, it->mTick, it->mLateral);
            it = mGems.erase(it);
        } else {
            ++it;
        }
    }
}
