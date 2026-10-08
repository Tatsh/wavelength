#pragma once

#include <vector>

#include "game/remixinfo.h"
#include "met/freqlist.h"
#include "script/dataarray.h"

/**
 * The list of the saved remixes, whose selection shows in the panel the description's `pic_panel`
 * names.
 *
 * The RTTI records the class as deriving from FreqList. The object is 0xe0 bytes and its vtable is
 * at `0x003cfa30`. A remix that cannot be chosen shows in grey. The destructor is
 * compiler-generated and has no declaration here.
 */
class RemixLoadList : public FreqList {
public:
    /**
     * Construct the list from its script description, listing no remix.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019ab10
     * @ghidraAddress PAL: 0x001a2060
     */
    RemixLoadList(DataArray *pData, const char *pszPanel);

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `remix_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x0035fe00
     * @ghidraAddress PAL: 0x003ce088
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new RemixLoadList(pData, pszPanel);
    }

    /**
     * Fill a row with the name of a remix, in grey when it cannot be chosen.
     *
     * @param nRow The row.
     * @param nItem The remix.
     * @ghidraAddress NTSC-U/C: 0x0019ac00
     * @ghidraAddress PAL: 0x001a2150
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor and show the selected remix in mPicPanel.
     *
     * @ghidraAddress NTSC-U/C: 0x0019ad80
     * @ghidraAddress PAL: 0x001a22d0
     */
    void UpdateCursor() override;

    /**
     * Replace the listed remixes and show them.
     *
     * @param remixes The remixes.
     * @param nGreyReadOnly Non-zero to grey the remixes that may not be changed.
     * @param nGreyUnplayable Non-zero to grey the remixes that cannot be played.
     * @ghidraAddress NTSC-U/C: 0x0019ab80
     * @ghidraAddress PAL: 0x001a20d0
     */
    void SetRemixes(const std::vector<RemixInfo> &remixes, int nGreyReadOnly, int nGreyUnplayable);

    std::vector<RemixInfo> mRemixes; /*!< The listed remixes. +0xc0 */
    const char *mPicPanel;           /*!< `pic_panel`, the panel of the selection. +0xd0 */
    int mGreyReadOnly;               /*!< Non-zero to grey the remixes that may not be changed. */
    int mGreyUnplayable;             /*!< Non-zero to grey the remixes that cannot be played. */
};
