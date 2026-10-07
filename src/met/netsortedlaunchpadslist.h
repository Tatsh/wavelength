#pragma once

#include "met/netmainlaunchpadslist.h"
#include "script/dataarray.h"

/**
 * List of launchpads sorted or found by the player. The chosen launchpad shows on the
 * `fn_sorted_pic` panel.
 *
 * The RTTI records the class as deriving from NetMainLaunchpadsList. Only the members its users
 * here need are declared.
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
    NetSortedLaunchpadsList(DataArray *pData, const char *pszPanel);
};
