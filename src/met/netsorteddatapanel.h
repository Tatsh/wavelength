#pragma once

#include <list>

#include "met/songpicpanel.h"
#include "netflow/netlaunchpadplayer.h"

/**
 * Panel that shows the song and the players of the online session chosen in the search results.
 *
 * The RTTI records the class as deriving from SongPicPanel, and its vtable is at `0x003cc968`.
 * Only the routines NetSortedScreen calls are declared, and the routines of the class are not
 * reconstructed.
 */
class NetSortedDataPanel : public SongPicPanel {
public:
    /**
     * Show the players of a session when it is the chosen one.
     *
     * @param pPlayers The players.
     * @param nLaunchpad The index of the session in the lobby list.
     * @ghidraAddress NTSC-U/C: 0x00176a28
     * @ghidraAddress PAL: 0x00179e88
     */
    void SetPlayers(std::list<NetLaunchpadPlayer> *pPlayers, int nLaunchpad);

    /**
     * Clear the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00176d28
     * @ghidraAddress PAL: 0x0017a188
     */
    void Reset();
};
