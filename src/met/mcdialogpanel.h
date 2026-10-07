#pragma once

#include "met/dialogpanel.h"
#include "script/dataarray.h"

/**
 * Dialog panel of a memory card screen, whose text includes the name of the memory card slot.
 *
 * The RTTI records the class as deriving from DialogPanel. The object is 0x100 bytes and its
 * vtable is at `0x003d16a8`. The metagame registers the class for the panel type
 * `mc_dialog_panel`, and the front-end description's `save_freq_dlg` panel is one. The panel must
 * show on an ErrorScreen.
 */
class MCDialogPanel : public DialogPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00362938
     */
    MCDialogPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003628e0
     * @ghidraAddress PAL: 0x003d0df8
     */
    ~MCDialogPanel() override;

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the entry type `mc_dialog_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00362890
     * @ghidraAddress PAL: 0x003d0da8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new MCDialogPanel(pData, pszDir);
    }

    /**
     * Start the entry as a DialogPanel, then show the text of the panel's name with the name of
     * the memory card slot.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a9db8
     * @ghidraAddress PAL: 0x001b1b18
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Show the text of the panel's name, formatted with the name of the memory card slot of the
     * current screen.
     *
     * @ghidraAddress NTSC-U/C: 0x001a9df0
     */
    virtual void ShowSlotText();
};
