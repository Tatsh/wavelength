#include "game/netarbiter.h"

#include "game/gamedb.h"
#include "game/localplayer.h"
#include "math/rand.h"
#include "msg/arbiterpacket.h"
#include "msg/freestylebumppacket.h"
#include "netflow/netinet.h"
#include "netflow/nettransport.h"
#include "os/memfun3command.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr int kNoTrack = -1;
constexpr int kNoVersion = -1;
constexpr int kNoPlayer = -1;

// The session order of the hosting console's player.
constexpr int kHostNetOrder = 0;

constexpr float kBroadcastIntervalMs = 200.0f;
constexpr float kSlowConnectionDelayMs = 100.0f;
constexpr float kDefaultDelayMs = 50.0f;

} // namespace

int NetArbiter::sVersion = 0;

NetArbiter::NetArbiter(GameTrackSelector *pGameTrackSelector)
    : mGameTrackSelector(pGameTrackSelector), mTrackSelector(pGameTrackSelector->GetNumTracks()),
      mLocalPlayer(kNoPlayer), mBroadcastCommand(NewMemFunCommand(this, &NetArbiter::Broadcast)),
      mVersions(pGameTrackSelector->GetNumPlayers(), 0) {
    mDelayMs = TheNetInet->GetConnectionType() == NetInet::kConnectionSlow ?
                   kSlowConnectionDelayMs :
                   kDefaultDelayMs;
    for (int i = 0; i < pGameTrackSelector->GetNumPlayers(); ++i) {
        const int nTrack = mGameTrackSelector->GetPlayerTrack(i);
        mTrackSelector.AddPlayer(nTrack, mGameTrackSelector->GetPlayerSlot(i));
        if (TheGameDb->IsLocalPlayer(i)) {
            mLocalPlayer = i;
        }
    }
}

void NetArbiter::Start() {
    Broadcast();
}

void NetArbiter::RequestTrack(int nPlayer, int nTrack) {
    const int nVersion = TheGameDb->mRuleSet == GameDb::kRuleSetRemix ?
                             LocalPlayer::sRemixerUpdateVersion :
                             LocalPlayer::sPlayerUpdateVersion;
    Command *pCommand =
        NewMemFun3Command(this, &NetArbiter::UpdatePlayer, nPlayer, nTrack, nVersion);
    TheSongScheduler.PostAfter(pCommand, mDelayMs, false);
}

void NetArbiter::UpdatePlayer(int nPlayer, int nTrack, int nVersion) {
    if (nVersion != kNoVersion) {
        mVersions[nPlayer] = nVersion;
    }

    bool bChanged = false;
    const int nLastTrack = mTrackSelector.GetNumTracks() - 1;
    if (nTrack == nLastTrack && mTrackSelector.GetNumPlayersOnTrack(nTrack) > 0) {
        // The last track takes one player, and a player moving onto it bumps the others off.
        for (int i = 0; i < mTrackSelector.GetNumPlayers(); ++i) {
            if (mTrackSelector.GetPlayerTrack(i) != nLastTrack || i == nPlayer) {
                continue;
            }
            const int nNewTrack = RandomInt(0, nLastTrack);
            if (TheGameDb->GetPlayerNetOrder(i) == kHostNetOrder) {
                bChanged = true;
                mTrackSelector.SetPlayerTrack(i, nNewTrack);
                mGameTrackSelector->MovePlayer(i, nNewTrack);
            } else {
                FreestyleBumpPacket packet(TheGameDb->GetPlayerNetOrder(i), nNewTrack);
                TheNetTransport->Send(packet);
            }
        }
    }

    if (nTrack != mTrackSelector.GetPlayerTrack(nPlayer)) {
        mTrackSelector.SetPlayerTrack(nPlayer, nTrack);
    }

    if (TheGameDb->GetPlayerNetOrder(nPlayer) == kHostNetOrder) {
        if (mGameTrackSelector->GetPlayerTrack(nPlayer) == nTrack) {
            const int nSlot = mGameTrackSelector->GetPlayerSlot(nPlayer);
            const int nArbiterSlot = mTrackSelector.GetPlayerSlot(nPlayer);
            if (nSlot != nArbiterSlot) {
                mGameTrackSelector->SwapPlayer(nPlayer, nTrack, nArbiterSlot);
            }
        }
        mGameTrackSelector->Resync();
    }

    if (bChanged) {
        Broadcast();
    }
}

void NetArbiter::Broadcast() {
    TheSongScheduler.Cancel(mBroadcastCommand.Get());

    ArbiterPacket packet(++sVersion);
    packet.mPlayers.reserve(mTrackSelector.GetNumPlayers());
    for (int i = 0; i < mTrackSelector.GetNumPlayers(); ++i) {
        const int nNetOrder = TheGameDb->GetPlayerNetOrder(i);
        const int nTrack = mTrackSelector.GetPlayerTrack(i);
        if (nTrack == kNoTrack) {
            continue;
        }
        const int nSlot = mTrackSelector.GetPlayerSlot(i);
        packet.mPlayers.push_back(
            ArbiterPacket::PlayerData{nNetOrder, mVersions[i], nTrack, nSlot});
    }
    if (!packet.mPlayers.empty()) {
        TheNetTransport->Send(packet);
    }

    TheSongScheduler.PostAfter(mBroadcastCommand.Get(), kBroadcastIntervalMs, false);
}

void NetArbiter::RemovePlayer(int nPlayer) {
    mTrackSelector.SetPlayerTrack(nPlayer, kNoTrack);
}

int NetArbiter::GetTrack(int nPlayer) {
    return mTrackSelector.GetPlayerTrack(nPlayer);
}
