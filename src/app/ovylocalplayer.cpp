#include "app/ovylocalplayer.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

OvyLocalPlayer::OvyLocalPlayer(int nPlayer, int nPosition, [[maybe_unused]] RndView *pLayout) {
    mPlayer = nPlayer;
    mPosition = nPosition;
    mPoints = nullptr;
    mRemix = nullptr;
    mReserved10[0] = 0;
    mReserved10[1] = 0;
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix && TheGameDb->IsLocalPlayer(nPlayer)) {
        auto *pLetterbox = dynamic_cast<RndTransformable *>(
            TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
        mRemix = std::make_unique<OvyRemixPanel>(nPlayer, nPosition, pLetterbox);
    }
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetGame) {
        mPoints = std::make_unique<HudPoints>(nPosition);
    }
}

OvyLocalPlayer::~OvyLocalPlayer() = default;

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
