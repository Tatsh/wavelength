#pragma once

#include <list>

#include "met/freqlist.h"
#include "netflow/netlaunchpadinfo.h"
#include "script/dataarray.h"

/**
 * List of the launchpads of the lobby.
 *
 * The RTTI records the class as deriving from FreqList. Only the members its users here need are
 * declared.
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
    NetMainLaunchpadsList(DataArray *pData, const char *pszPanel);

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
};
