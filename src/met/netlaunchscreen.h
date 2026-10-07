#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The screen the host shows while the song of an online session starts.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003cd9e8`. The
 * metagame registers the class for the screen type `net_launch_screen`.
 */
class NetLaunchScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00187fc8
     * @ghidraAddress PAL: 0x0018c208
     */
    explicit NetLaunchScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_launch_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035bc70
     * @ghidraAddress PAL: 0x003c97c8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetLaunchScreen(pData);
    }

    /**
     * Route the end of the entry, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00188000
     * @ghidraAddress PAL: 0x0018c240
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035bcb0
     * @ghidraAddress PAL: 0x003c9808
     */
    const char *Title() override {
        return "";
    }

    /**
     * Start the song once the screen has entered, and share the remix of a remix game.
     *
     * Without a session, or for another screen, the lost session dialog shows.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00188068
     * @ghidraAddress PAL: 0x0018c2a8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
