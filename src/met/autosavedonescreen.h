#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that reports the end of the automatic save after a song, and shows the next queued
 * unlock once it has entered.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003ce900`. The destructor at `0x0035e358` is compiler-generated and has no declaration
 * here.
 */
class AutoSaveDoneScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001960d0
     * @ghidraAddress PAL: 0x0019d4a0
     */
    explicit AutoSaveDoneScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e318
     * @ghidraAddress PAL: 0x003cc3f8
     */
    static UIScreen *New(DataArray *pData) {
        return new AutoSaveDoneScreen(pData);
    }

    /**
     * Route the end of a transition, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00196108
     * @ghidraAddress PAL: 0x0019d4d8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the next queued unlock.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00196170
     * @ghidraAddress PAL: 0x0019d540
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
