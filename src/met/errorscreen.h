#pragma once

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Screen of a dialog about the memory card or another error, between the screen that opened it
 * and the screen it leads to.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0xa0 bytes and its vtable
 * is at `0x003d1630`. The metagame registers the class for the screen type `error_screen`, and
 * the front-end description's `cur_freq_not_saved` screen is one. The description's
 * `start_screen` and `done_screen` give the two screens. The memory card screens derive from the
 * class.
 */
class ErrorScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001a9ec0
     * @ghidraAddress PAL: 0x001b1ce0
     */
    explicit ErrorScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362970
     * @ghidraAddress PAL: 0x003d0e50
     */
    ~ErrorScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `error_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00354fa8
     * @ghidraAddress PAL: 0x003c2258
     */
    static UIScreen *New(DataArray *pData) {
        return new ErrorScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003629d8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Set the screen the dialog leads to.
     *
     * @param pszScreen The screen.
     * @ghidraAddress NTSC-U/C: 0x00358ac0
     * @ghidraAddress PAL: 0x003d0f58
     */
    virtual void SetDoneScreen(const char *pszScreen) {
        mDoneScreen = pszScreen;
    }

    /**
     * Set the screen that opened the dialog.
     *
     * @param pszScreen The screen.
     * @ghidraAddress NTSC-U/C: 0x00358ae0
     * @ghidraAddress PAL: 0x003d0f78
     */
    virtual void SetStartScreen(const char *pszScreen) {
        mStartScreen = pszScreen;
    }

    /**
     * Choose the memory card slot the dialog is about.
     *
     * @param nSlot The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x00358b00
     */
    virtual void SetSlot(int nSlot) {
        mSlot = nSlot;
    }

    /**
     * Report the localised name of the memory card slot the dialog is about.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x001a9f88
     * @ghidraAddress PAL: 0x001b1da8
     */
    const char *GetSlotName();

    /**
     * Show the dialog of a memory card failure with retry, continue, and cancel buttons.
     *
     * @param nStatus The failure.
     * @ghidraAddress NTSC-U/C: 0x001a9fb0
     * @ghidraAddress PAL: 0x001b1ea8
     */
    void ShowCardError(int nStatus);

    /**
     * Show the dialog of a memory card failure with retry and cancel buttons.
     *
     * @param nStatus The failure.
     * @param nOperation The operation that failed: 1 for a save, 2 for a copy, and 3 for a
     *                   deletion.
     * @ghidraAddress NTSC-U/C: 0x001aa490
     * @ghidraAddress PAL: 0x001b2418
     */
    void ShowCardErrorTwoOption(int nStatus, int nOperation);

    /**
     * Show the `no_space_error` dialog of a memory card without enough space.
     *
     * @param nSpace The space the save needs.
     * @ghidraAddress NTSC-U/C: 0x001aa9e8
     */
    void ShowNoSpaceError(int nSpace);

    String mDoneScreen;  /*!< The screen the dialog leads to, `done_screen`. +0x70 */
    String mStartScreen; /*!< The screen that opened the dialog, `start_screen`. +0x84 */
    int mSlot;           /*!< The memory card slot the dialog is about. +0x98 */
};
