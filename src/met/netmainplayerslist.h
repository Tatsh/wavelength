#pragma once

#include <list>

#include "met/freqlist.h"
#include "netflow/lobbyplayer.h"
#include "script/dataarray.h"

/**
 * List of the players of the lobby.
 *
 * The RTTI records the class as deriving from FreqList. Only the members its users here need are
 * declared.
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
     * Replace the players and select one.
     *
     * The name is inferred.
     *
     * @param pPlayers The players to copy.
     * @param nSelected The player to select, or -1 for the current selection.
     * @ghidraAddress NTSC-U/C: 0x0019bbf0
     * @ghidraAddress PAL: 0x001a3928
     */
    void SetPlayers(std::list<LobbyPlayer> *pPlayers, int nSelected);
};
