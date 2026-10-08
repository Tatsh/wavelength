#include "app/ovylocalplayer.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "os/string.h"
#include "rnd/manager.h"

OvyLocalPlayer::OvyLocalPlayer(int nPlayer, int nPosition) {
    mPlayer = nPlayer;
    mPosition = nPosition;
    mPoints = nullptr;
    mRemix = nullptr;
    mReserved10[0] = 0;
    mReserved10[1] = 0;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix && TheGameDb->IsLocalPlayer(nPlayer)) {
        auto *pLetterbox = dynamic_cast<Rnd::Transformable *>(
            Rnd::TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
        mRemix = new OvyRemixPanel(nPlayer, nPosition, pLetterbox);
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        mPoints = new HudPoints(nPosition);
    }
}

OvyLocalPlayer::~OvyLocalPlayer() {
    delete mPoints;
    delete mRemix;
}

void OvyLocalPlayer::Poll([[maybe_unused]] float fUnused,
                          [[maybe_unused]] float fUnused2,
                          float fSongDelta,
                          float fRealDelta) {
    if (mPoints != nullptr) {
        mPoints->Poll(fSongDelta, fRealDelta);
    }
    if (mRemix != nullptr) {
        mRemix->Poll(fRealDelta);
    }
}

void OvyLocalPlayer::DrawHud() {
    if (mRemix != nullptr) {
        mRemix->Draw();
    }
}

void OvyLocalPlayer::DrawAvatar() {
}

void OvyLocalPlayer::Reset() {
    if (mPoints != nullptr) {
        mPoints->Reset();
    }
}
