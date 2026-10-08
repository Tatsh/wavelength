#include "met/netrankedplayerspanel.h"

#include "netflow/netlobby.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/text.h"

namespace {

constexpr char kRankTextFormat[] = "fn_main_rank_%02d.txt";
constexpr char kNameTextFormat[] = "fn_main_rank_freq_%02d.txt";
constexpr char kNewbieToken[] = "newbie";
constexpr char kRankFormat[] = "%2d";
constexpr char kNoText[] = "";

constexpr int kTopOfRanking = 0;
constexpr int kTopTenMode = 1;

Rnd::Text *FindRowText(const char *pszFormat, int nRow) {
    return dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(FormatString(pszFormat, nRow)));
}

} // namespace

void NetRankedPlayersPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    for (int nRow = 1; nRow <= kNumRows; ++nRow) {
        FindRowText(kRankTextFormat, nRow)->SetText(kNoText);
        FindRowText(kNameTextFormat, nRow)->SetText(kNoText);
    }
    TheNetLobby->RequestRanks(this, kNumRows, kTopOfRanking, kTopTenMode);
}

bool NetRankedPlayersPanel::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nLobbyPlayersMsgType) {
        return HandleLobbyPlayers(static_cast<LobbyPlayersMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool NetRankedPlayersPanel::HandleLobbyPlayers(LobbyPlayersMsg *pMsg) {
    if (!mLoaded) {
        return false;
    }

    std::list<LobbyPlayer> *pPlayers = pMsg->mPlayers;
    int nRow = 0;
    for (auto player = pPlayers->begin(); player != pPlayers->end() && nRow < kNumRows; ++player) {
        ++nRow;
        Rnd::Text *pRank = FindRowText(kRankTextFormat, nRow);
        if (player->IsNewbie()) {
            pRank->SetText(TheLocale.Localize(kNewbieToken, true));
        } else {
            pRank->SetText(FormatString(kRankFormat, player->mRank));
        }
        FindRowText(kNameTextFormat, nRow)->SetText(player->mName.c_str());
    }
    return false;
}
