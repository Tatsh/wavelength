#pragma once

#include "met/netmainplayerslist.h"
#include "script/dataarray.h"

/**
 * List of the ranking, with the details and the Freq of the selected player on `fn_rank_p`.
 * Scrolling past either end requests the previous or the next page from NetRankingScreen.
 *
 * The RTTI records the class as deriving from NetMainPlayersList. Its vtable is at `0x003cf790`.
 */
class NetRankedPlayersList : public NetMainPlayersList {
public:
    /**
     * Construct a list from its script description.
     *
     * The square button does not mute in this list.
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

    /**
     * Clear the Freq of `fn_rank_p` and destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x0019bf00
     * @ghidraAddress PAL: 0x001a3c38
     */
    ~NetRankedPlayersList() override;

    /**
     * Show the rank and the name of a player in a row, or clear the row past the last player.
     *
     * @param nRow The row.
     * @param nItem The player.
     * @ghidraAddress NTSC-U/C: 0x0019c130
     * @ghidraAddress PAL: 0x001a3e68
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor and show the details and the Freq of the selected player.
     *
     * @ghidraAddress NTSC-U/C: 0x0019c2f0
     * @ghidraAddress PAL: 0x001a4028
     */
    void UpdateCursor() override;

    /**
     * Show or hide the cursor and the text `fn_rank_p_01.txt`.
     *
     * @param bSelected Whether the cursor shows the selected state.
     * @ghidraAddress NTSC-U/C: 0x0019c260
     * @ghidraAddress PAL: 0x001a3f98
     */
    void SetCursorSelected(bool bSelected) override;

    /**
     * Select the previous player, or request the previous page from the first player.
     *
     * @ghidraAddress NTSC-U/C: 0x0019c010
     * @ghidraAddress PAL: 0x001a3d48
     */
    void ScrollUp() override;

    /**
     * Select the next player, or request the next page from the last player.
     *
     * @ghidraAddress NTSC-U/C: 0x0019c098
     * @ghidraAddress PAL: 0x001a3dd0
     */
    void ScrollDown() override;
};
