#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * A screen whose panels hide while a dialog it opens shows.
 *
 * The RTTI records the class as deriving from FreqScreen. The class has no members and no
 * instances of its own. SetupRemixSaveScreen derives from it.
 */
class NeedsDialogScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * Inline. The constructors of the derived classes expand it.
     *
     * @param pData The script description.
     */
    explicit NeedsDialogScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Show or hide every panel of the screen except `help` and `title`. Vtable slot 11.
     *
     * The name is inferred.
     *
     * @param bShowing Whether the panels show.
     * @ghidraAddress NTSC-U/C: 0x001ae958
     * @ghidraAddress PAL: 0x001b7638
     */
    virtual void SetPanelsShowing(bool bShowing);

    /**
     * Go on from the screen. Vtable slot 12.
     *
     * The name is inferred.
     */
    virtual void Proceed() = 0;
};
