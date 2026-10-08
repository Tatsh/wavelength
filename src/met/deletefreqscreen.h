#pragma once

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that deletes a saved Freq from the memory card once it has entered.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0e50` and, for MemcardUser, `0x003d0dc0`. Deleting the Freq the first
 * player uses in the first slot replaces that player with a default `player_1`.
 */
class DeleteFreqScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description, with no Freq.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ac620
     * @ghidraAddress PAL: 0x001b4c80
     */
    explicit DeleteFreqScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363878
     * @ghidraAddress PAL: 0x003d1c08
     */
    ~DeleteFreqScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363998
     */
    static UIScreen *New(DataArray *pData) {
        return new DeleteFreqScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003639d8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ac6f0
     * @ghidraAddress PAL: 0x001b4e40
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Go to the done screen after the deletion, or open the dialog of a failure of the memory
     * card.
     *
     * @param nStatus How the deletion ended, one of MemcardTask::Status.
     * @ghidraAddress NTSC-U/C: 0x001ac690
     */
    void OnFreqDeleted(int nStatus) override;

    String mFreqName; /*!< The name of the Freq to delete. */

private:
    /**
     * Start the deletion once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ac758
     * @ghidraAddress PAL: 0x001b4ea8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
