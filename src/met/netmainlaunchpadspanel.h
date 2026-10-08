#pragma once

#include <list>

#include "met/netmainpanel.h"
#include "msg/lobbylaunchpadsmsg.h"
#include "netflow/lobbyplayer.h"
#include "netflow/netlaunchpadinfo.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uipanel.h"

/**
 * Panel of the online main screen that lists the launchpads of the lobby, with the host and up to
 * four players of the selected launchpad.
 *
 * The RTTI records the class as deriving from NetMainPanel. Its vtable is at `0x003ccce0`. The
 * cross button on the cursor goes to join the selected launchpad.
 */
class NetMainLaunchpadsPanel : public NetMainPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00173408
     * @ghidraAddress PAL: 0x00176850
     */
    NetMainLaunchpadsPanel(DataArray *pData, const char *pszDir)
        : NetMainPanel(pData, pszDir), mLaunchpad(0) {
    }

    /**
     * Create a panel from its script description.
     *
     * Metagame::RegisterScreenClasses() registers the routine for the entry type
     * `fn_main_launchpads_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00358310
     * @ghidraAddress PAL: 0x003c56c0
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetMainLaunchpadsPanel(pData, pszDir);
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00358208
     * @ghidraAddress PAL: 0x003c55b8
     */
    ~NetMainLaunchpadsPanel() override {
    }

    /**
     * Route the launchpads and the choice to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00173ab8
     * @ghidraAddress PAL: 0x00176f18
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Take the focus and move the cursor of the list.
     *
     * @ghidraAddress NTSC-U/C: 0x00173490
     * @ghidraAddress PAL: 0x001768d8
     */
    void Focus() override;

    /**
     * Lose the focus, dim the cursor of the list, and hide the players.
     *
     * @ghidraAddress NTSC-U/C: 0x001734f8
     * @ghidraAddress PAL: 0x00176940
     */
    void Unfocus() override;

    /**
     * Request the launchpads of the lobby.
     *
     * @ghidraAddress NTSC-U/C: 0x00173440
     * @ghidraAddress PAL: 0x00176888
     */
    void RequestUpdate() override;

    /**
     * Hide the host and the rows of the players.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00173568
     * @ghidraAddress PAL: 0x001769b0
     */
    void HidePlayers();

    /**
     * Show the song or the remix of a launchpad as its host, and remember the launchpad.
     *
     * The name is inferred.
     *
     * @param pLaunchpad The launchpad.
     * @ghidraAddress NTSC-U/C: 0x00173730
     * @ghidraAddress PAL: 0x00176b78
     */
    void ShowLaunchpad(NetLaunchpadInfo *pLaunchpad);

    /**
     * Show the rank icons and the names of the players of the shown launchpad, or request the
     * players again when they belong to another launchpad.
     *
     * The name is inferred.
     *
     * @param pPlayers The players.
     * @param nLaunchpad The launchpad of the players.
     * @ghidraAddress NTSC-U/C: 0x001738c0
     * @ghidraAddress PAL: 0x00176d20
     */
    void ShowPlayers(std::list<LobbyPlayer> *pPlayers, int nLaunchpad);

private:
    /**
     * Fill the list while the panel is loaded, and request the launchpads again later.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00173b48
     * @ghidraAddress PAL: 0x00176fa8
     */
    bool HandleLaunchpads(LobbyLaunchpadsMsg *pMsg);

    /**
     * Go to join the selected launchpad when the cross button chooses `cursor`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00173c48
     * @ghidraAddress PAL: 0x001770a8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    int mLaunchpad; // The identifier of the launchpad ShowLaunchpad() showed.
};
