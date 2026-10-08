#include "app/ovyallplayer.h"

#include "game/gamedb.h"

OvyAllPlayer::OvyAllPlayer(int nPlayer, int nIndex, Rnd::View *pHudView) {
    mPlayer = nPlayer;
    mScore = nullptr;
    mDuelScore = nullptr;
    mAvatar = nullptr;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        mScore = new OvyScore(nPlayer, nIndex, pHudView);
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mDuelScore = new OvyDuelScore(nIndex);
    }
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo ||
        TheGameDb->mRuleSet == GameDb::kRuleSetDuel ||
        (TheGameDb->IsLocalPlayer(nPlayer) && TheGameDb->mRuleSet == GameDb::kRuleSetRemix &&
         TheGameDb->mCommunity == GameDb::kCommunityOnline)) {
        mAvatar = new OvyAvatar(nPlayer, nIndex, pHudView);
    }
}

OvyAllPlayer::~OvyAllPlayer() {
    delete mAvatar;
    delete mScore;
    delete mDuelScore;
}

void OvyAllPlayer::Poll(float fSongDelta, float fRealDelta, float fTickDelta) {
    if (mScore != nullptr) {
        mScore->Poll(fSongDelta, fRealDelta, fTickDelta);
    }
    if (mDuelScore != nullptr) {
        mDuelScore->Poll();
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
