#pragma once

#include <list>

#include "game/avatarpartset.h"
#include "met/freqlist.h"
#include "msg/joypadinputmsg.h"
#include "netflow/lobbyplayer.h"
#include "script/dataarray.h"

/**
 * List of the players of the lobby, with the details of the selected player on `fn_main_play`.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x003cf838`. The square
 * button mutes or unmutes the chat of the selected player.
 */
class NetMainPlayersList : public FreqList {
public:
    /**
     * Construct a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019b5f0
     * @ghidraAddress PAL: 0x001a3328
     */
    NetMainPlayersList(DataArray *pData, const char *pszPanel);

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `players_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x00360230
     * @ghidraAddress PAL: 0x003ce748
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new NetMainPlayersList(pData, pszPanel);
    }

    /**
     * Destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x00360138
     * @ghidraAddress PAL: 0x003ce650
     */
    ~NetMainPlayersList() override {
    }

    /**
     * Show the rank icon, the name, and the mute indicator of a player in a row.
     *
     * @param nRow The row.
     * @param nItem The player.
     * @ghidraAddress NTSC-U/C: 0x0019b678
     * @ghidraAddress PAL: 0x001a33b0
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor and show the rank, the connection, and the games of the selected player.
     *
     * A player whose rank is not yet known is marked as requested, and the panel `fn_main_play`
     * listens for the rank.
     *
     * @ghidraAddress NTSC-U/C: 0x0019b8d8
     * @ghidraAddress PAL: 0x001a3610
     */
    void UpdateCursor() override;

    /**
     * Show or hide the cursor and the details of the selected player.
     *
     * @param bSelected Whether the cursor shows the selected state.
     * @ghidraAddress NTSC-U/C: 0x0019b750
     * @ghidraAddress PAL: 0x001a3488
     */
    void SetCursorSelected(bool bSelected) override;

    /**
     * Mute or unmute the selected player with the square button, then handle the button as a list
     * does.
     *
     * The player of this console cannot be muted.
     *
     * @param pMsg The message of the button.
     * @return The result of UIList::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0019bdf0
     * @ghidraAddress PAL: 0x001a3b28
     */
    bool HandleJoypad(JoypadInputMsg *pMsg) override;

    /**
     * Replace the players and select one.
     *
     * The Freq of `fn_rank_p` is cleared. The name is inferred.
     *
     * @param pPlayers The players to copy.
     * @param nSelected The player to select, or -1 for the current selection.
     * @ghidraAddress NTSC-U/C: 0x0019bbf0
     * @ghidraAddress PAL: 0x001a3928
     */
    void SetPlayers(std::list<LobbyPlayer> *pPlayers, int nSelected);

    /**
     * Record the rank of a player, and show it when the player is selected.
     *
     * The name is inferred.
     *
     * @param nAccount The account of the player.
     * @param nRank The rank.
     * @ghidraAddress NTSC-U/C: 0x0019bce8
     * @ghidraAddress PAL: 0x001a3a20
     */
    void SetRank(int nAccount, int nRank);

    /**
     * Report the Freq of the selected player.
     *
     * The name is inferred.
     *
     * @return The Freq, or null when the list is empty.
     * @ghidraAddress NTSC-U/C: 0x0019bd80
     * @ghidraAddress PAL: 0x001a3ab8
     */
    AvatarPartSet *SelectedAvatar();

protected:
    int mMuteEnabled;                // Whether the square button mutes the selected player.
    std::list<LobbyPlayer> mPlayers; // The players of the list.
};
