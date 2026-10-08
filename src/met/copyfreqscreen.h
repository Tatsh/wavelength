#pragma once

#include "game/campaign.h"
#include "met/savefreqscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that saves a copy of a Freq to the memory card under the Freq's name.
 *
 * The RTTI records the class as deriving from SaveFreqScreen. Its vtables are at `0x003d0f58` and,
 * for MemcardUser, `0x003d0ec8`. The screen that opens it sets the Freq to copy.
 */
class CopyFreqScreen : public SaveFreqScreen {
public:
    /**
     * Construct the screen from its script description, with a default Freq.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ac470
     * @ghidraAddress PAL: 0x001b4ad0
     */
    explicit CopyFreqScreen(DataArray *pData) : SaveFreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363790
     */
    ~CopyFreqScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363830
     */
    static UIScreen *New(DataArray *pData) {
        return new CopyFreqScreen(pData);
    }

    /**
     * Route the end of the entry to HandleTransitionComplete(), and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ac558
     * @ghidraAddress PAL: 0x001b4bb8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter without the check for cheats SaveFreqScreen makes.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001ac4b8
     * @ghidraAddress PAL: 0x001b4b18
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Go to the done screen after the copy, open the dialog of a failure of the memory card, or
     * leave the other failures to SaveFreqScreen.
     *
     * @param nStatus How the copy ended, one of MemcardTask::Status.
     * @param nSpace The space the copy needs, when the memory card is too full.
     * @ghidraAddress NTSC-U/C: 0x001ac4d8
     * @ghidraAddress PAL: 0x001b4b38
     */
    void OnFreqSaved(int nStatus, int nSpace) override;

    Campaign mProfile; /*!< The Freq to copy. */

private:
    /**
     * Start the copy once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ac5c0
     * @ghidraAddress PAL: 0x001b4c20
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
