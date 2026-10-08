#pragma once

#include "met/netmainplayerslist.h"
#include "script/dataarray.h"

/**
 * List of the ranking. Scrolling past either end requests the previous or the next page from
 * NetRankingScreen.
 *
 * The RTTI records the class as deriving from NetMainPlayersList. Only the members its users here
 * need are declared.
 */
class NetRankedPlayersList : public NetMainPlayersList {
public:
    /**
     * Construct a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019bfd8
     * @ghidraAddress PAL: 0x001a3d10
     */
    NetRankedPlayersList(DataArray *pData, const char *pszPanel);

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `ranked_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x00360338
     * @ghidraAddress PAL: 0x003ce850
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new NetRankedPlayersList(pData, pszPanel);
    }
};
