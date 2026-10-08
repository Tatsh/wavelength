#pragma once

#include "met/netmainlaunchpadslist.h"
#include "script/dataarray.h"

/**
 * List of launchpads sorted or found by the player. The chosen launchpad shows on the
 * `fn_sorted_pic` panel, and scrolling past either end requests the previous or the next page
 * from NetSortedLaunchpadsPanel.
 *
 * The RTTI records the class as deriving from NetMainLaunchpadsList. Its vtable is at `0x003cf598`.
 */
class NetSortedLaunchpadsList : public NetMainLaunchpadsList {
public:
    /**
     * Construct a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019cff8
     * @ghidraAddress PAL: 0x001a4d10
     */
    NetSortedLaunchpadsList(DataArray *pData, const char *pszPanel)
        : NetMainLaunchpadsList(pData, pszPanel) {
    }

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `sorted_launchpads_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x00360740
     * @ghidraAddress PAL: 0x003cec58
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new NetSortedLaunchpadsList(pData, pszPanel);
    }

    /**
     * Destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x00360618
     * @ghidraAddress PAL: 0x003ceb30
     */
    ~NetSortedLaunchpadsList() override {
    }

    /**
     * Move the cursor, show the selected launchpad on `fn_sorted_pic`, and request its players.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d030
     * @ghidraAddress PAL: 0x001a4d48
     */
    void UpdateCursor() override;

    /**
     * Select the previous launchpad, or request the previous page from the first one.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d108
     * @ghidraAddress PAL: 0x001a4e20
     */
    void ScrollUp() override;

    /**
     * Select the next launchpad, or request the next page from the last one.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d158
     * @ghidraAddress PAL: 0x001a4e70
     */
    void ScrollDown() override;
};
