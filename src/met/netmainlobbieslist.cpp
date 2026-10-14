#include "met/netmainlobbieslist.h"

#include <iterator>

#include "met/netswitchlobbyscreen.h"
#include "os/joypad.h"
#include "os/listat.h"
#include "os/string.h"
#include "ui/uimanager.h"

namespace {

constexpr char kSwitchScreen[] = "net_switch_lobby";
constexpr char kPlayersFormat[] = "%d/%d";

// The cells of a row.
enum Column {
    kColumnName = 0,
    kColumnPlayers = 1,
};

} // namespace

void NetMainLobbiesList::UpdateRow(int nRow, int nItem) {
    const NetChatroomInfo &chatroom = *ListAt(mChatrooms, nItem);
    SetCellText(nRow, kColumnName, chatroom.mName.c_str());
    SetCellText(nRow,
                kColumnPlayers,
                FormatString(kPlayersFormat, chatroom.mPlayerCount, chatroom.mMaxPlayers));
}

void NetMainLobbiesList::SetChatrooms(std::list<NetChatroomInfo> *pChatrooms) {
    mChatrooms = *pChatrooms;
    Refresh(static_cast<int>(pChatrooms->size()), kKeepSelection);
}

bool NetMainLobbiesList::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mButton == kPadCross && pMsg->mPressed != 0) {
        auto *pScreen =
            dynamic_cast<NetSwitchLobbyScreen *>(TheUI.FindScreen(kSwitchScreen, false));
        pScreen->mChatroomId = ListAt(mChatrooms, mSelected)->mId;
    }
    return UIList::HandleJoypad(pMsg);
}
