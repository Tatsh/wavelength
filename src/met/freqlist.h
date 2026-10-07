#pragma once

#include "script/dataarray.h"
#include "ui/uilist.h"

/**
 * A list component of the Freq menus, which plays the menu sounds as the selection moves.
 *
 * The RTTI records the class as deriving from UIList. The object is 0xc0 bytes and its vtable is
 * at `0x003cfb80`. The destructor at `0x0035fa98` is compiler-generated and has no declaration
 * here.
 */
class FreqList : public UIList {
public:
    /**
     * Construct the list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019a178
     * @ghidraAddress PAL: 0x001a16b8
     */
    FreqList(DataArray *pData, const char *pszPanel);

    /**
     * Select the previous item, and play the sound of the up button when the selection moved.
     *
     * @ghidraAddress NTSC-U/C: 0x0019a1b0
     * @ghidraAddress PAL: 0x001a16f0
     */
    void ScrollUp() override;

    /**
     * Select the next item, and play the sound of the down button when the selection moved.
     *
     * @ghidraAddress NTSC-U/C: 0x0019a1f8
     * @ghidraAddress PAL: 0x001a1738
     */
    void ScrollDown() override;
};
