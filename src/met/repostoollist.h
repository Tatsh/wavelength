#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <list>

#include "met/freqlist.h"
#include "netflow/netreporemix.h"
#include "script/dataarray.h"

/**
 * The list of the remixes of the player's repository on the repository tool screen, whose
 * selection shows in the panel the description's `pic_panel` names.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x004661a8`.
 */
class ReposToolList : public FreqList {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x003ce220
     */
    ~ReposToolList() override = default;

    /**
     * Construct the list from its script description, listing no remix.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress PAL: 0x001a2b10
     */
    ReposToolList(DataArray *pData, const char *pszPanel);

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `repos_tool_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress PAL: 0x003ce318
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new ReposToolList(pData, pszPanel);
    }

    /**
     * Replace the listed remixes and show them, keeping the selection.
     *
     * @param remixes The remixes.
     * @ghidraAddress PAL: 0x001a2bb0
     */
    void SetRemixes(const std::list<NetRepoRemix> &remixes);

    /**
     * Fill a row with the name of a remix.
     *
     * @param nRow The row.
     * @param nItem The remix.
     * @ghidraAddress PAL: 0x001a2c28
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor and show the details, the song, and the note of the selected remix in
     * mPicPanel.
     *
     * @ghidraAddress PAL: 0x001a2cb8
     */
    void UpdateCursor() override;

    std::list<NetRepoRemix> mRemixes; /*!< The listed remixes. */
    const char *mPicPanel;            /*!< `pic_panel`, the panel of the selection. */
};

#endif
