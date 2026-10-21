#include "app/ovyallplayer.h"

#include "game/gamedb.h"

OvyAllPlayer::OvyAllPlayer(int nPlayer, int nIndex, RndView *pHudView) {
    mPlayer = nPlayer;
    mScore = nullptr;
    mDuelScore = nullptr;
    mAvatar = nullptr;
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetGame) {
        mScore = std::make_unique<OvyScore>(nPlayer, nIndex, pHudView);
    } else if (TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel) {
        mDuelScore = std::make_unique<OvyDuelScore>(nIndex, pHudView);
    }
    if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo ||
        TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel ||
        (TheGameDb->IsLocalPlayer(nPlayer) && TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix &&
         TheGameDb->GetCommunity() == GameDb::kCommunityOnline)) {
        mAvatar = std::make_unique<OvyAvatar>(nPlayer, nIndex, pHudView);
    }
}

OvyAllPlayer::~OvyAllPlayer() = default;

void OvyAllPlayer::Poll(float fSongDelta, float fRealDelta, float fTickDelta) {
    if (mScore != nullptr) {
        mScore->Poll(fSongDelta, fRealDelta, fTickDelta);
    }
    if (mDuelScore != nullptr) {
        mDuelScore->Poll(fSongDelta, fRealDelta);
    }
    if (mAvatar != nullptr) {
        mAvatar->Poll(fRealDelta);
    }
}

void OvyAllPlayer::DrawHud() {
    if (mScore != nullptr) {
        mScore->Draw();
    }
}

void OvyAllPlayer::DrawAvatar() {
    if (mAvatar != nullptr) {
        mAvatar->Draw();
    }
}

void OvyAllPlayer::Reset() {
    if (mScore != nullptr) {
        mScore->Reset();
    }
    if (mDuelScore != nullptr) {
        mDuelScore->Reset();
    }
    if (mAvatar != nullptr) {
        mAvatar->Reset();
    }
}
