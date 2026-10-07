#include "met/netdoplayersearchscreen.h"

#include <cstring>

#include "met/playersearchresultsscreen.h"
#include "netflow/netlobby.h"
#include "ui/uimanager.h"

namespace {

constexpr char kSearchErrorScreen[] = "search_player_error";
constexpr char kOfflineResultsScreen[] = "fn_find_player_off";
constexpr char kOnlineResultsScreen[] = "fn_find_player_on";

constexpr int kNoChatroom = -1;

} // namespace

NetDoPlayerSearchScreen::NetDoPlayerSearchScreen(DataArray *pData) : FreqScreen(pData) {
}

bool NetDoPlayerSearchScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nFindPlayerMsgType) {
        return HandleFindPlayer(static_cast<FindPlayerMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetDoPlayerSearchScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheNetLobby->FindPlayer(this, mPlayerName.c_str());
    }
    return false;
}

bool NetDoPlayerSearchScreen::HandleFindPlayer(FindPlayerMsg *pMsg) {
    if (pMsg->mResult != 0) {
        TheUI.GotoScreen(kSearchErrorScreen);
        return false;
    }

    const String unused(""); // Yes, the binary builds and discards this string.
    const NetChatroomInfo chatroom(pMsg->mChatroom);
    const char *pszScreen =
        chatroom.mId == kNoChatroom ? kOfflineResultsScreen : kOnlineResultsScreen;
    PlayerSearchResultsScreen *pResults =
        dynamic_cast<PlayerSearchResultsScreen *>(TheUI.FindScreen(pszScreen, false));
    pResults->SetPlayer(pMsg->mPlayer, chatroom);
    TheUI.GotoScreen(pResults);
    return false;
}
