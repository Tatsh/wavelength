#pragma once

#include <list>

#include "met/netbuttonpanel.h"
#include "met/netlpadpanel.h"
#include "netflow/netlaunchpadplayer.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel of the buttons of the launchpad screen, which enables `launch` once every player is ready
 * and `share` once a remix can be shared.
 *
 * The RTTI records the class as deriving from NetButtonPanel and from NetLPadPanel at `+0x100`.
 * Its vtable is at `0x003ccbd0`. The meshes always take the `sub` materials.
 */
class NetHostLPadButtonPanel : public NetButtonPanel, public NetLPadPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001740c8
     * @ghidraAddress PAL: 0x00177528
     */
    NetHostLPadButtonPanel(DataArray *pData, const char *pszDir) : NetButtonPanel(pData, pszDir) {
    }

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00358448
     * @ghidraAddress PAL: 0x003c57f8
     */
    ~NetHostLPadButtonPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetHostLPadButtonPanel(pData, pszDir);
    }

    /**
     * Take the focus, and show the focused materials once the panel has loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00174108
     * @ghidraAddress PAL: 0x00177568
     */
    void Focus() override;

    /**
     * Show the unfocused materials.
     *
     * @ghidraAddress NTSC-U/C: 0x001742c0
     * @ghidraAddress PAL: 0x00177720
     */
    void Unfocus() override;

    /**
     * Enable `launch` and `share` for the players of the session, and move the focus off a button
     * that becomes disabled.
     *
     * Without a focused button the focus goes to `edit` for the host and to `data` for a guest.
     *
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x00174468
     * @ghidraAddress PAL: 0x001778c8
     */
    void Update(std::list<NetLaunchpadPlayer> *pPlayers) override;
};
