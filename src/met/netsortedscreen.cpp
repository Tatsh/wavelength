#include "met/netsortedscreen.h"

#include <cstring>

#include "met/netsorteddatapanel.h"
#include "met/netsortedlaunchpadspanel.h"
#include "ui/uimanager.h"

namespace {

constexpr char kListPanel[] = "fn_sorted";
constexpr char kDataPanel[] = "fn_sorted_pic";

} // namespace

NetSortedScreen::NetSortedScreen(DataArray *pData) : FreqScreen(pData) {
}

void NetSortedScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    static_cast<NetSortedLaunchpadsPanel *>(TheUI.FindPanel(kListPanel, false))
        ->SetSearch(mArena.c_str(), mRuleSet, mSkillLevel);
    FreqScreen::Enter(pPrevScreen, fTime);
    static_cast<NetSortedDataPanel *>(TheUI.FindPanel(kDataPanel, false))->Reset();
}

void NetSortedScreen::SetSearch(const char *pszArena, int nRuleSet, int nSkillLevel) {
    mArena = pszArena;
    mSkillLevel = nSkillLevel;
    mRuleSet = nRuleSet;
}

bool NetSortedScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nLobbyPlayersMsgType) {
        return HandleLobbyPlayers(static_cast<LobbyPlayersMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetSortedScreen::HandleLobbyPlayers(LobbyPlayersMsg *pMsg) {
    if (TheUI.mCurrentScreen != nullptr && strcmp(TheUI.mCurrentScreen->mName, kListPanel) == 0) {
        static_cast<NetSortedDataPanel *>(TheUI.FindPanel(kDataPanel, false))
            ->SetPlayers(pMsg->mPlayers, pMsg->mLaunchpad);
    }
    return false;
}
