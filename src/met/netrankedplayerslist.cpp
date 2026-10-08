#include "met/netrankedplayerslist.h"

#include <cstdio>
#include <cstring>
#include <iterator>

#include "met/avatarpanel.h"
#include "met/metagameutil.h"
#include "met/netrankingscreen.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/view.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDetailPanel[] = "fn_rank_p";
constexpr char kRankingScreen[] = "fn_rank";
constexpr char kCursorText[] = "fn_rank_p_01.txt";
constexpr char kDetailView[] = "fn_rank_p_text.view";
constexpr char kRankMesh[] = "fn_rank_p_rank.mesh";
constexpr char kNameComponent[] = "name";
constexpr char kRankComponent[] = "rank";
constexpr char kGamesComponent[] = "games";
constexpr char kNetComponent[] = "net";
constexpr char kLoginComponent[] = "login";
constexpr char kNewbieToken[] = "newbie";
constexpr char kRankToken[] = "fn_rank_p_02";
constexpr char kGamesToken[] = "fn_rank_p_03";
constexpr char kNetToken[] = "fn_rank_p_04";
constexpr char kLoginToken[] = "fn_rank_p_05";
constexpr char kRankFormat[] = "%d";
constexpr char kRowRankFormat[] = "%3d";
constexpr char kNoText[] = "";

// The cells of a row.
enum Column {
    kColumnRank = 0,
    kColumnName = 1,
};

// The size of the text of a rank.
constexpr int kRankTextSize = 32;

AvatarPanel *FindAvatarPanel() {
    return dynamic_cast<AvatarPanel *>(TheUI.FindPanel(kDetailPanel, false));
}

NetRankingScreen *FindRankingScreen() {
    return dynamic_cast<NetRankingScreen *>(TheUI.FindScreen(kRankingScreen, false));
}

} // namespace

NetRankedPlayersList::NetRankedPlayersList(DataArray *pData, const char *pszPanel)
    : NetMainPlayersList(pData, pszPanel) {
    mMuteEnabled = 0;
}

NetRankedPlayersList::~NetRankedPlayersList() {
    AvatarPanel *pPanel = FindAvatarPanel();
    if (pPanel != nullptr) {
        pPanel->SetAvatar(nullptr);
    }
}

void NetRankedPlayersList::ScrollUp() {
    if (mSelected == 0) {
        FindRankingScreen()->RequestPage(NetRankingScreen::kPagePrevious);
    } else {
        FreqList::ScrollUp();
    }
}

void NetRankedPlayersList::ScrollDown() {
    if (mSelected < mItemCount - 1) {
        FreqList::ScrollDown();
    } else {
        FindRankingScreen()->RequestPage(NetRankingScreen::kPageNext);
    }
}

void NetRankedPlayersList::UpdateRow(int nRow, int nItem) {
    if (static_cast<unsigned int>(nItem) < mPlayers.size()) {
        const LobbyPlayer &player = *std::next(mPlayers.begin(), nItem);
        if (player.IsNewbie()) {
            SetCellText(nRow, kColumnRank, TheLocale.Localize(kNewbieToken, true));
        } else {
            SetCellText(nRow, kColumnRank, FormatString(kRowRankFormat, player.mRank));
        }
        SetCellText(nRow, kColumnName, player.mName.c_str());
    } else {
        SetCellText(nRow, kColumnName, kNoText);
        SetCellText(nRow, kColumnRank, kNoText);
    }
}

void NetRankedPlayersList::SetCursorSelected(bool bSelected) {
    UIList::SetCursorSelected(bSelected);
    dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kCursorText))->SetShowing(bSelected);
}

void NetRankedPlayersList::UpdateCursor() {
    UIList::UpdateCursor();
    auto *pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kDetailView));
    if (mPlayers.empty()) {
        pView->SetShowing(false);
        return;
    }
    pView->SetShowing(true);

    UIComponent *pName = TheUI.FindComponent(kDetailPanel, kNameComponent, false);
    LobbyPlayer &player = *std::next(mPlayers.begin(), mSelected);
    pName->SetText(player.mName.c_str());

    UIComponent *pRank = TheUI.FindComponent(kDetailPanel, kRankComponent, false);
    char szRank[kRankTextSize];
    if (player.IsNewbie()) {
        std::strcpy(szRank, TheLocale.Localize(kNewbieToken, true));
    } else if (player.mRank > 0) {
        std::sprintf(szRank, kRankFormat, player.mRank);
    } else {
        szRank[0] = kNoText[0];
    }
    FindAvatarPanel()->SetAvatar(&player.mAvatar);
    pRank->SetText(FormatString(TheLocale.Localize(kRankToken, true), szRank));

    UIComponent *pGames = TheUI.FindComponent(kDetailPanel, kGamesComponent, false);
    pGames->SetText(FormatString(TheLocale.Localize(kGamesToken, true), player.mGames));

    UIComponent *pNet = TheUI.FindComponent(kDetailPanel, kNetComponent, false);
    pNet->SetText(FormatString(TheLocale.Localize(kNetToken, true),
                               GetConnectionTypeName(player.mConnectionType)));

    UIComponent *pLogin = TheUI.FindComponent(kDetailPanel, kLoginComponent, false);
    String login;
    player.mLastOnline.FormatShortDate(login);
    pLogin->SetText(FormatString(TheLocale.Localize(kLoginToken, true), login.c_str()));

    dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(kRankMesh))
        ->SetMat(FindRankMaterial(player.mRankIcon));
}
