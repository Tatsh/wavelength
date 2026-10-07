#pragma once

#include "met/freqpanel.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Panel whose meshes change material while it has the focus.
 *
 * The RTTI records the class as deriving from FreqPanel. Its vtable is at `0x003cc8c0`. With the
 * focus, the panel mesh `<name>_panel.mesh` takes `panel_<prefix>_hi.mat` and the background
 * `<name>_bg.mesh` takes `bg_hi.mat`. Without the focus they take `panel_<prefix>.mat` and
 * `bg_no.mat`. The prefix is the description's `focus_material_prefix`, `sub` by default.
 */
class FocusChangePanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the panel's files.
     * @ghidraAddress NTSC-U/C: 0x0016ef40
     * @ghidraAddress PAL: 0x00172190
     */
    FocusChangePanel(DataArray *pData, const char *pszDir);

    /**
     * Take the focus, and show the focused materials once the panel has loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x0016efb8
     * @ghidraAddress PAL: 0x00172208
     */
    void Focus() override;

    /**
     * Show the unfocused materials once the panel has loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x0016f178
     * @ghidraAddress PAL: 0x001723c8
     */
    void Unfocus() override;

    String mMaterialPrefix; /*!< The `focus_material_prefix` entry. */
};
