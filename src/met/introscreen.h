#pragma once

#include "msg/message.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * A screen that shows its panels for a fixed time and then moves on to another screen.
 *
 * The RTTI includes the name and records UIScreen as the one base. The object is 0x48 bytes. The
 * metagame registers the class for the screen type `intro_screen`. The start-up flow shows two of
 * them, `sony_pres` with the presented-by notice and `hmx_logo`, each with a `hold_time` and a
 * `next_screen`.
 */
class IntroScreen : public UIScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * The `next_screen` entry is required. The `hold_time` entry is optional and defaults to one
     * second.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018c158
     * @ghidraAddress PAL: 0x00192910
     */
    explicit IntroScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `intro_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035cf88
     * @ghidraAddress PAL: 0x003cb060
     */
    static UIScreen *New(DataArray *pData) {
        return new IntroScreen(pData);
    }

    /**
     * Start the hold once the move to the screen has finished, and pass every other message to
     * UIScreen.
     *
     * @param pMsg The message.
     * @return False for the end of the move, and what UIScreen reports for any other message.
     * @ghidraAddress NTSC-U/C: 0x0018c320
     * @ghidraAddress PAL: 0x00192e58
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, and move to the next screen once the hold has run out.
     *
     * @param fTime The front-end time.
     * @ghidraAddress NTSC-U/C: 0x0018c1e0
     * @ghidraAddress PAL: 0x00192998
     */
    void Poll(float fTime) override;

private:
    /**
     * Start the hold.
     *
     * @param pMsg The message that reported the end of the move.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0018c2a0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    // The system time the hold ends at, or -1 while no hold runs. +0x3c
    float mEndTime;
    // The hold in milliseconds. +0x40
    float mHoldMs;
    // The screen that follows. +0x44
    const char *mNextScreen;
};
