#pragma once

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that loads the remix GameDb describes from the memory card into the buffer of the remix
 * data, then leads to the done screen.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0718` and, for MemcardUser, `0x003d0688`. The front-end description's
 * `load_remix` screen is one. A failure opens `load_remix_not_found_error` or `load_failed_error`,
 * which retry the load or return to the start screen.
 */
class LoadRemixScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ae048
     * @ghidraAddress PAL: 0x001b6ab8
     */
    explicit LoadRemixScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003642c0
     */
    ~LoadRemixScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003643d8
     */
    static UIScreen *New(DataArray *pData) {
        return new LoadRemixScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00364418
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ae1d8
     * @ghidraAddress PAL: 0x001b6d38
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Go to the done screen after a load, or open the dialog of the failure.
     *
     * @param nStatus How the load ended, one of MemcardTask::Status.
     * @ghidraAddress NTSC-U/C: 0x001ae088
     * @ghidraAddress PAL: 0x001b6bd8
     */
    void OnRemixLoaded(int nStatus) override;

private:
    /**
     * Start the load once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ae240
     * @ghidraAddress PAL: 0x001b6da0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
