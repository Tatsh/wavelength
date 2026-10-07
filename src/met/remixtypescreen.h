#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The remix menu that starts a new remix, loads one, or plays the remix tutorial.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003ce7e0`. The
 * metagame registers the class for the screen type `remix_type_screen`. Its buttons are `new`,
 * `load`, and `training`.
 */
class RemixTypeScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00188ab8
     * @ghidraAddress PAL: 0x0018f050
     */
    explicit RemixTypeScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `remix_type_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c278
     * @ghidraAddress PAL: 0x003ca350
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixTypeScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00188af0
     * @ghidraAddress PAL: 0x0018f088
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00188b58
     * @ghidraAddress PAL: 0x0018f0f0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set up the remix of the button chosen with the cross button.
     *
     * `new` describes an untitled remix by the current players, `load` marks a remix to load, and
     * `training` sets up and starts the remix tutorial.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00188b78
     * @ghidraAddress PAL: 0x0018f110
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
