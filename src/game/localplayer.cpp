#include "game/localplayer.h"

#include "game/gamedb.h"
#include "game/mixer.h"
#include "msg/multiplierpacket.h"
#include "msg/playerupdatepacket.h"
#include "msg/remixerupdatepacket.h"
#include "netflow/nettransport.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr float kBroadcastIntervalMs = 500.0f;

} // namespace

int LocalPlayer::sPlayerUpdateVersion = 0;

int LocalPlayer::sRemixerUpdateVersion = 0;

LocalPlayer::LocalPlayer(int nIndex, int nTicksPerBar, int nPadNum)
    : Player(nIndex, nTicksPerBar), mPadNum(nPadNum),
      mBroadcastCommand(NewMemFunCommand(this, &LocalPlayer::Broadcast)) {
}

LocalPlayer::~LocalPlayer() {
}

void LocalPlayer::SetTrack(Track *pTrack) {
    if (mTrack != nullptr) {
        TheMixer->RemovePlayer(mTrack->mIndex);
    }
    Player::SetTrack(pTrack);
    TheMixer->AddPlayer(pTrack->mIndex);
    Broadcast();
}

void LocalPlayer::SetScore(int nScore) {
    Player::SetScore(nScore);
    Broadcast();
}

void LocalPlayer::AddScore(int nPoints) {
    Player::AddScore(nPoints);
    Broadcast();
}

void LocalPlayer::Capture() {
    Player::CommitPendingPoints(true);
    Broadcast();
}

void LocalPlayer::SetCatching(bool bCatching) {
    if (bCatching != mCatching) {
        Player::SetCatching(bCatching);
        Broadcast();
    }
}

void LocalPlayer::ActivateMultiplier() {
    Player::ActivateMultiplier();
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        MultiplierPacket packet;
        TheNetTransport->Send(packet);
    }
}

void LocalPlayer::SetRepeat(bool bRepeat) {
    Player::SetRepeat(bRepeat);
    Broadcast();
}

void LocalPlayer::Broadcast() {
    if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
        return;
    }
    TheSongScheduler.Cancel(mBroadcastCommand.Get());

    const int nRuleSet = TheGameDb->mRuleSet;
    if (nRuleSet == GameDb::kRuleSetGame || nRuleSet == GameDb::kRuleSetDuel) {
        PlayerUpdatePacket packet(mTrack->mIndex, mScore, mCatching, ++sPlayerUpdateVersion);
        TheNetTransport->Send(packet);
    } else if (nRuleSet == GameDb::kRuleSetRemix) {
        RemixerUpdatePacket packet(mTrack->mIndex, mRepeat, ++sRemixerUpdateVersion);
        TheNetTransport->Send(packet);
    }

    TheSongScheduler.PostAfter(mBroadcastCommand.Get(), kBroadcastIntervalMs, false);
}
