#pragma once

#include "memcard/memcarduser.h"
#include "met/overwritesavescreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Memory card screen that saves the remix GameDb describes under a name once it has entered, such
 * as `save_remix` of the front-end description.
 *
 * The RTTI records the class as deriving from OverwriteSaveScreen and from MemcardUser at `+0xb0`.
 * Its vtables are at `0x003d0a30` and, for MemcardUser, `0x003d09a0`. A remix of the name leads to
 * `remix_exists_error`, and a card with as many remixes as allowed to `remix_limit_error`.
 */
class SaveRemixScreen : public OverwriteSaveScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description, with no name.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ad768
     */
    explicit SaveRemixScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363e48
     */
    ~SaveRemixScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363f88
     */
    static UIScreen *New(DataArray *pData) {
        return new SaveRemixScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00363fc8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ad9a0
     * @ghidraAddress PAL: 0x001b6318
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Go to the done screen after the save, or open the dialog of the failure.
     *
     * @param nStatus How the save ended, one of MemcardTask::Status.
     * @param nNeeded The space the save needs, when the memory card is too full.
     * @ghidraAddress NTSC-U/C: 0x001ad7e0
     */
    void OnRemixSaved(int nStatus, int nNeeded) override;

    String mRemixName; /*!< The name the remix is saved under. */

private:
    /**
     * Save the remix under mRemixName once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ada08
     * @ghidraAddress PAL: 0x001b6380
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
