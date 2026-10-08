#pragma once

#include <list>

#include "met/songpicpanel.h"
#include "netflow/lobbyplayer.h"
#include "netflow/netlaunchpadinfo.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel that shows the song, the genre, and up to four players of the launchpad chosen on a
 * sorted list.
 *
 * The RTTI records the class as deriving from SongPicPanel.
 */
class NetSortedDataPanel : public SongPicPanel {
public:
    /** The number of player rows. */
    static constexpr int kNumPlayerRows = 4;

    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001768d8
     * @ghidraAddress PAL: 0x00179d38
     */
    NetSortedDataPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003588f8
     * @ghidraAddress PAL: 0x003c5ca8
     */
    ~NetSortedDataPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00354fe8
     * @ghidraAddress PAL: 0x003c2298
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetSortedDataPanel(pData, pszDir);
    }

    /**
     * Hide the player rows.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00176910
     * @ghidraAddress PAL: 0x00179d70
     */
    void HidePlayers();

    /**
     * Show the players of the shown launchpad, or request them again when the list belongs to
     * another launchpad.
     *
     * @param pPlayers The players.
     * @param nLaunchpad The launchpad the players belong to.
     * @ghidraAddress NTSC-U/C: 0x00176a28
     * @ghidraAddress PAL: 0x00179e88
     */
    void SetPlayers(std::list<LobbyPlayer> *pPlayers, int nLaunchpad);

    /**
     * Show the genre and the band picture of a launchpad's song, or nothing for an empty entry.
     *
     * The name is inferred.
     *
     * @param pLaunchpad The launchpad.
     * @ghidraAddress NTSC-U/C: 0x00176bf0
     * @ghidraAddress PAL: 0x0017a050
     */
    void ShowLaunchpad(NetLaunchpadInfo *pLaunchpad);

    /**
     * Hide the players, the genre, and the picture.
     *
     * @ghidraAddress NTSC-U/C: 0x00176d28
     * @ghidraAddress PAL: 0x0017a188
     */
    void Reset();

    int mLaunchpad; /*!< The identifier of the launchpad shown. */
};
