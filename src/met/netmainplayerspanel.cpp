#include "met/netmainplayerspanel.h"

#include "met/metagame.h"
#include "met/metagameutil.h"
#include "met/netmainplayerslist.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlobby.h"
#include "os/locale.h"
#include "os/system.h"
#include "rnd/manager.h"

namespace {

constexpr char kAvatarMesh[] = "fn_main_play_freq.mesh";
constexpr char kListComponent[] = "list";
constexpr char kFocusHelpToken[] = "fn_main_players_focus_HELP";
constexpr char kHelpToken[] = "fn_main_players_HELP";

// The selection that makes SetPlayers() keep the current one.
constexpr int kCurrentSelection = -1;

// The milliseconds after a reply before the players are requested again.
constexpr float kRequestIntervalMs = 20000.0f;

} // namespace

void NetMainPlayersPanel::FinishLoad() {
    NetMainPanel::FinishLoad();
    mAvatarMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(kAvatarMesh));
    mAvatarMesh->SetShowing(false);
}

void NetMainPlayersPanel::Enter(bool bForce, float fTime) {
    mShowAvatar = 1;
    NetMainPanel::Enter(bForce, fTime);
}

void NetMainPlayersPanel::Exit(bool bForce, float fTime) {
    mShowAvatar = 0;
    NetMainPanel::Exit(bForce, fTime);
}

void NetMainPlayersPanel::RequestUpdate() {
    TheNetLobby->RequestLobbyPlayers(this);
}

void NetMainPlayersPanel::Focus() {
    if (!mLoaded) {
        return;
    }
    FocusChangePanel::Focus();
    mHilite = 1;
    if (mLoaded) { // Yes, the binary checks the load again.
        static_cast<NetMainPlayersList *>(FindComponent(kListComponent, false))->UpdateCursor();
    }
    TheMetagame.SetHelpText(TheLocale.Localize(kFocusHelpToken, true));
}

void NetMainPlayersPanel::Unfocus() {
    if (!mLoaded) {
        return;
    }
    FocusChangePanel::Unfocus();
    mHilite = 0;
    static_cast<NetMainPlayersList *>(FindComponent(kListComponent, false))
        ->SetCursorSelected(false);
    TheMetagame.SetHelpText(TheLocale.Localize(kHelpToken, true));
}

void NetMainPlayersPanel::Draw() {
    FreqPanel::Draw();
    AvatarPartSet *pAvatar =
        static_cast<NetMainPlayersList *>(FindComponent(kListComponent, false))->SelectedAvatar();
    if (pAvatar != nullptr && mShowAvatar != 0) {
        DrawAvatarOnMesh(pAvatar, mAvatarMesh);
    }
}

bool NetMainPlayersPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLobbyPlayersMsgType) {
        return HandleLobbyPlayers(static_cast<LobbyPlayersMsg *>(pMsg));
    }
    if (nType == g_nPlayerRankMsgType) {
        return HandlePlayerRank(static_cast<PlayerRankMsg *>(pMsg));
    }
    return NetMainPanel::DispatchPriv(pMsg);
}

bool NetMainPlayersPanel::HandleLobbyPlayers(LobbyPlayersMsg *pMsg) {
    if (mLoaded) {
        static_cast<NetMainPlayersList *>(FindComponent(kListComponent, false))
            ->SetPlayers(pMsg->mPlayers, kCurrentSelection);
        HideFlash();
    }
    mRequestTime = SystemMs() + kRequestIntervalMs;
    return false;
}

bool NetMainPlayersPanel::HandlePlayerRank(PlayerRankMsg *pMsg) {
    if (mLoaded) {
        static_cast<NetMainPlayersList *>(FindComponent(kListComponent, false))
            ->SetRank(pMsg->mAccount, pMsg->mRank);
    }
    return false;
}
