#pragma once

#include "met/freqpanel.h"
#include "os/string.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Panel of a dialog box, whose text and buttons come from the locale and whose box grows with the
 * text.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0x100 bytes and its vtable
 * is at `0x003d0048`. The metagame registers the class for the panel type `dialog_panel`. Every
 * dialog loads the shared `dialog.rnd` of its directory and the `dialog_...` objects of the file,
 * and shows the text of its name. A `tri_cancel` entry shows the cancel hint of the triangle
 * button.
 */
class DialogPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001a4698
     * @ghidraAddress PAL: 0x001ac388
     */
    DialogPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001a48d0
     * @ghidraAddress PAL: 0x001ac5c0
     */
    ~DialogPanel() override;

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the entry type `dialog_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003617f0
     * @ghidraAddress PAL: 0x003cfd08
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new DialogPanel(pData, pszDir);
    }

    /**
     * Start the entry, hide the unused rows, and show the text that waits, or the text of the
     * panel's name.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a4938
     * @ghidraAddress PAL: 0x001ac628
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Finish loading as the shared `dialog` objects, focus the `focus` component, and find the
     * views that size the box.
     *
     * @ghidraAddress NTSC-U/C: 0x001a4758
     * @ghidraAddress PAL: 0x001ac448
     */
    void FinishLoad() override;

    /**
     * Show a text and size the box for it and the buttons, or store the text for the entry while
     * the panel loads.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x001a4bc8
     * @ghidraAddress PAL: 0x001ac8b8
     */
    void SetText(const char *pszText);

    int mTriCancel;         /*!< Non-zero when the cancel hint shows, `tri_cancel`. +0xe0 */
    Rnd::View *mPanelView;  /*!< The view `dialog_panel.view`, whose frame sets the height. */
    Rnd::View *mButtonView; /*!< The view `dialog_text.view`, whose frame places the buttons. */
    String mPendingText;    /*!< The text SetText() received before the panel loaded. +0xec */
};
