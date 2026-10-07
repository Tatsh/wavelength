#include "game/track.h"

#include "game/player.h"
#include "game/stats.h"

namespace {

constexpr int kNoPlayer = -1;

} // namespace

Track::Track(int nIndex) : mIndex(nIndex), mPlayer(nullptr) {
}

void Track::SetPlayer(Player *pPlayer) {
    mPlayer = pPlayer;
    TheStats->SetTrackPlayer(mIndex, pPlayer ? pPlayer->GetIndex() : kNoPlayer);
}
