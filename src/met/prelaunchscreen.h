#pragma once

#include "met/transitionscreen.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Transition screen that changes to the screen its description's `launch_name` names once it has
 * entered.
 *
 * The RTTI records the class as deriving from TransitionScreen. The object is 0x80 bytes and its
 * vtable is at `0x003ce960`. The destructor at `0x0035e270` is compiler-generated and has no
 * declaration here.
 */
class PreLaunchScreen : public TransitionScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00195d28
     * @ghidraAddress PAL: 0x0019d0f8
     */
    explicit PreLaunchScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e230
     * @ghidraAddress PAL: 0x003cc310
     */
    static UIScreen *New(DataArray *pData) {
        return new PreLaunchScreen(pData);
    }

    /**
     * Route the end of a transition, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00195d80
     * @ghidraAddress PAL: 0x0019d150
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Change to mLaunchName once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00195de8
     * @ghidraAddress PAL: 0x0019d1b8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    const char *mLaunchName; /*!< `launch_name`, the screen to change to. */
};
