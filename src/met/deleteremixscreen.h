#pragma once

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that deletes a saved remix from the memory card once it has entered.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0928` and, for MemcardUser, `0x003d0898`.
 */
class DeleteRemixScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description, with no remix.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001adaf8
     * @ghidraAddress PAL: 0x001b6470
     */
    explicit DeleteRemixScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363fe0
     * @ghidraAddress PAL: 0x003d2758
     */
    ~DeleteRemixScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00364100
     */
    static UIScreen *New(DataArray *pData) {
        return new DeleteRemixScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00364140
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001adbc8
     * @ghidraAddress PAL: 0x001b65d8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Go to the done screen after the deletion, or open the dialog of a failure of the memory
     * card.
     *
     * @param nStatus How the deletion ended, one of MemcardTask::Status.
     * @ghidraAddress NTSC-U/C: 0x001adb68
     */
    void OnRemixDeleted(int nStatus) override;

    String mRemixName; /*!< The name of the remix to delete. */

private:
    /**
     * Start the deletion once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001adc30
     * @ghidraAddress PAL: 0x001b6640
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
