#include "met/netrankingscreen.h"

#include <list>

#include "met/fullrankedpanel.h"
#include "netflow/netlobby.h"
#include "rnd/mesh.h"
#include "ui/uimanager.h"

namespace {

constexpr char kRankPanel[] = "fn_rank";
constexpr char kListComponent[] = "list";

constexpr int kPageSize = 10;
constexpr int kRankingMode = 0;
constexpr int kSelectFirst = 0;
constexpr int kSelectLast = 1000;
constexpr int kNoSelection = -1;

FullRankedPanel *FindRankPanel() {
    return static_cast<FullRankedPanel *>(TheUI.FindPanel(kRankPanel, false));
}

void HideArrows(UIList *pList) {
    pList->mUpArrow->SetShowing(false);
    pList->mShowUpArrow = false;
    pList->mDownArrow->SetShowing(false);
    pList->mShowDownArrow = false;
}

} // namespace

NetRankingScreen::NetRankingScreen(DataArray *pData) : FreqScreen(pData) {
    mSelectFirst = 1;
    mLoading = 0;
    mHasPrevious = 0;
}

void NetRankingScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mList =
        static_cast<NetRankedPlayersList *>(TheUI.FindComponent(kRankPanel, kListComponent, false));
    mList->Refresh(0, UIList::kKeepSelection);
    TheNetLobby->RequestRanks(this, kPageSize, kPageTop, kRankingMode);
    mSelectFirst = 1;
    mHasPrevious = 0;
    FindRankPanel()->StartFlash(TheUI.mTime, false);
    mLoading = 1;
    HideArrows(mList);
}

bool NetRankingScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nLobbyPlayersMsgType) {
        return HandleLobbyPlayers(static_cast<LobbyPlayersMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void NetRankingScreen::RequestPage(int nPage) {
    if (nPage == kPagePrevious && mHasPrevious == 0) {
        return;
    }
    {
        std::list<LobbyPlayer> empty;
        mList->SetPlayers(&empty, kNoSelection);
    }
    mSelectFirst = nPage != kPagePrevious;
    FindRankPanel()->StartFlash(TheUI.mTime, false);
    mLoading = 1;
    TheNetLobby->RequestRanks(this, kPageSize, nPage, kRankingMode);
    HideArrows(mList);
}

bool NetRankingScreen::HandleLobbyPlayers(LobbyPlayersMsg *pMsg) {
    if (TheUI.mCurrentScreen != this || mNextScreen != nullptr) {
        return false;
    }
    mHasPrevious = pMsg->mHasPrevious;
    mList->SetPlayers(pMsg->mPlayers, mSelectFirst != 0 ? kSelectFirst : kSelectLast);
    mLoading = 0;
    FindRankPanel()->HideFlash();
    mList->mUpArrow->SetShowing(mHasPrevious != 0);
    mList->mShowUpArrow = mHasPrevious != 0;
    mList->mDownArrow->SetShowing(true);
    mList->mShowDownArrow = true;
    return false;
}
