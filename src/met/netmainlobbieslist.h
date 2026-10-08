#pragma once

#include <list>

#include "met/freqlist.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netchatroominfo.h"
#include "script/dataarray.h"

/**
 * List of the chat rooms of the lobby, with the number of players of each.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x003cf6e8`. The cross
 * button chooses the room NetSwitchLobbyScreen joins.
 */
class NetMainLobbiesList : public FreqList {
public:
    /**
     * Construct a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019c760
     * @ghidraAddress PAL: 0x001a4498
     */
    NetMainLobbiesList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
    }

    /**
     * Destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x00360388
     * @ghidraAddress PAL: 0x003ce8a0
     */
    ~NetMainLobbiesList() override {
    }

    /**
     * Show the name and the players of a chat room in a row.
     *
     * @param nRow The row.
     * @param nItem The chat room.
     * @ghidraAddress NTSC-U/C: 0x0019c7e0
     * @ghidraAddress PAL: 0x001a4518
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Choose the selected room for NetSwitchLobbyScreen with the cross button, then handle the
     * button as a list does.
     *
     * @param pMsg The message of the button.
     * @return The result of UIList::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0019c8e0
     * @ghidraAddress PAL: 0x001a4618
     */
    bool HandleJoypad(JoypadInputMsg *pMsg) override;

    /**
     * Replace the chat rooms and keep the selection.
     *
     * The name is inferred.
     *
     * @param pChatrooms The chat rooms to copy.
     * @ghidraAddress NTSC-U/C: 0x0019c868
     * @ghidraAddress PAL: 0x001a45a0
     */
    void SetChatrooms(std::list<NetChatroomInfo> *pChatrooms);

private:
    std::list<NetChatroomInfo> mChatrooms; // The chat rooms of the list.
};
