#include "met/netmainplayerslist.h"

#include <cstdio>
#include <cstring>
#include <iterator>

#include "met/avatarpanel.h"
#include "met/metagameutil.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDetailPanel[] = "fn_main_play";
constexpr char kRankComponent[] = "rank";
constexpr char kNetComponent[] = "net";
constexpr char kGamesComponent[] = "games";
constexpr char kRankMesh[] = "fn_main_play_rank_sel.mesh";
constexpr char kAvatarPanel[] = "fn_rank_p";
constexpr char kMuteToken[] = "mute_indicator";
constexpr char kNewbieToken[] = "newbie";
constexpr char kRankToken[] = "fn_main_play_rank";
constexpr char kNetToken[] = "fn_main_play_net";
constexpr char kGamesToken[] = "fn_main_play_games";
constexpr char kRankFormat[] = "%d";
constexpr char kNoText[] = "";

// The cells of a row.
enum Column {
    kColumnRank = 0,
    kColumnName = 1,
    kColumnMute = 2,
};

// The rank of a player whose rank the list has not requested, and of one it has requested.
constexpr int kRankUnknown = 0;
constexpr int kRankRequested = -1;

// The selection SetPlayers() passes on to keep the current one.
constexpr int kNoSelection = -1;

// The size of the text of a rank.
constexpr int kRankTextSize = 32;

} // namespace

NetMainPlayersList::NetMainPlayersList(DataArray *pData, const char *pszPanel)
    : FreqList(pData, pszPanel), mMuteEnabled(1) {
}

void NetMainPlayersList::UpdateRow(int nRow, int nItem) {
    const LobbyPlayer &player = *std::next(mPlayers.begin(), nItem);
    SetCellMat(nRow, kColumnRank, FindRankMaterial(player.mRankIcon));
    SetCellText(nRow, kColumnName, player.mName.c_str());
    if (player.mMuted != 0) {
        SetCellText(nRow, kColumnMute, TheLocale.Localize(kMuteToken, true));
    } else {
        SetCellText(nRow, kColumnMute, kNoText);
    }
}

void NetMainPlayersList::SetCursorSelected(bool bSelected) {
    UIList::SetCursorSelected(bSelected);
    if (static_cast<unsigned int>(mSelected) < mPlayers.size()) {
        TheUI.FindComponent(kDetailPanel, kRankComponent, false)->SetShowing(bSelected);
        TheUI.FindComponent(kDetailPanel, kNetComponent, false)->SetShowing(bSelected);
        TheUI.FindComponent(kDetailPanel, kGamesComponent, false)->SetShowing(bSelected);
    }
    dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(kRankMesh))->SetShowing(bSelected);
}

void NetMainPlayersList::UpdateCursor() {
    UIList::UpdateCursor();
    if (mPlayers.empty()) {
        return;
    }

    LobbyPlayer &player = *std::next(mPlayers.begin(), mSelected);
    UIComponent *pRank = TheUI.FindComponent(kDetailPanel, kRankComponent, false);
    char szRank[kRankTextSize];
    if (player.IsNewbie()) {
        std::strcpy(szRank, TheLocale.Localize(kNewbieToken, true));
    } else {
        if (player.mRank > 0) {
            std::sprintf(szRank, kRankFormat, player.mRank);
        } else {
            szRank[0] = kNoText[0];
        }
        if (player.mRank == kRankUnknown) {
            player.mRank = kRankRequested;
            TheNetLobby->AddLadderListener(TheUI.FindPanel(kDetailPanel, false), player.mId);
        }
    }
    pRank->SetText(FormatString(TheLocale.Localize(kRankToken, true), szRank));

    UIComponent *pNet = TheUI.FindComponent(kDetailPanel, kNetComponent, false);
    pNet->SetText(FormatString(TheLocale.Localize(kNetToken, true),
                               GetConnectionTypeName(player.mConnectionType)));

    UIComponent *pGames = TheUI.FindComponent(kDetailPanel, kGamesComponent, false);
    pGames->SetText(FormatString(TheLocale.Localize(kGamesToken, true), player.mGames));

    dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(kRankMesh))
        ->SetMat(FindRankMaterial(player.mRankIcon));
}

void NetMainPlayersList::SetPlayers(std::list<LobbyPlayer> *pPlayers, int nSelected) {
    dynamic_cast<AvatarPanel *>(TheUI.FindPanel(kAvatarPanel, false))->SetAvatar(nullptr);
    mPlayers = *pPlayers;
    Refresh(static_cast<int>(pPlayers->size()),
            nSelected > kNoSelection ? nSelected : kKeepSelection);
}

void NetMainPlayersList::SetRank(int nAccount, int nRank) {
    int nItem = 0;
    for (auto &player : mPlayers) {
        if (player.mId == nAccount) {
            player.mRank = nRank;
            if (nItem == mSelected) {
                UpdateCursor();
            }
            return;
        }
        ++nItem;
    }
}

AvatarPartSet *NetMainPlayersList::SelectedAvatar() {
    if (mPlayers.empty()) {
        return nullptr;
    }
    return &std::next(mPlayers.begin(), mSelected)->mAvatar;
}

bool NetMainPlayersList::HandleJoypad(JoypadInputMsg *pMsg) {
    if (mMuteEnabled != 0 && pMsg->mButton == kPadSquare && pMsg->mPressed != 0) {
        LobbyPlayer &player = *std::next(mPlayers.begin(), mSelected);
        if (TheNetLobby->GetAccountId() != player.mId) {
            FxMidi::PlaySquare();
            player.mMuted ^= 1;
            TheNetLobby->SetMuted(player.mId, player.mMuted != 0);
            UpdateRow(mCursorRow, mSelected);
        } else {
            FxMidi::PlayWrong();
        }
    }
    return UIList::HandleJoypad(pMsg);
}
