#pragma once

#include <list>

#include "met/freqlist.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netlaunchpadinfo.h"
#include "script/dataarray.h"

/**
 * List of the launchpads of the lobby, with the host and the players of the selected one on the
 * list's panel.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x003cf640`. The cross
 * button chooses an open launchpad for NetJoinLPadScreen.
 */
class NetMainLaunchpadsList : public FreqList {
public:
    /**
     * Construct a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019c9a8
     * @ghidraAddress PAL: 0x001a46e0
     */
    NetMainLaunchpadsList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
    }

    /**
     * Destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x003604d0
     * @ghidraAddress PAL: 0x003ce9e8
     */
    ~NetMainLaunchpadsList() override {
    }

    /**
     * Show the status, the mode, the difficulty, the connection, the power-ups, and the artist of a
     * launchpad in a row, in grey for a launchpad that cannot be joined and in cyan for a remix.
     *
     * @param nRow The row.
     * @param nItem The launchpad.
     * @ghidraAddress NTSC-U/C: 0x0019cab8
     * @ghidraAddress PAL: 0x001a47f0
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor, show the selected launchpad on the list's panel, and request its players.
     *
     * @ghidraAddress NTSC-U/C: 0x0019cdd0
     * @ghidraAddress PAL: 0x001a4ae8
     */
    void UpdateCursor() override;

    /**
     * Choose the selected launchpad for NetJoinLPadScreen with the cross button, then handle the
     * button as a list does.
     *
     * @param pMsg The message of the button.
     * @return False when the cross button finds no open launchpad, otherwise the result of
     * UIList::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0019ced0
     * @ghidraAddress PAL: 0x001a4be8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg) override;

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `launchpads_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x003605c8
     * @ghidraAddress PAL: 0x003ceae0
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new NetMainLaunchpadsList(pData, pszPanel);
    }

    /**
     * Replace the launchpads and select one.
     *
     * The name is inferred.
     *
     * @param pLaunchpads The launchpads to copy.
     * @param nSelected The launchpad to select, or -1 for the current selection.
     * @ghidraAddress NTSC-U/C: 0x0019ca28
     * @ghidraAddress PAL: 0x001a4760
     */
    void SetLaunchpads(std::list<NetLaunchpadInfo> *pLaunchpads, int nSelected);

protected:
    std::list<NetLaunchpadInfo> mLaunchpads; // The launchpads of the list.
};
