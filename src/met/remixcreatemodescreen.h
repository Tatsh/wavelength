#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The menu that chooses between modifying the remix of a song and starting from its default.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003ce538`. The
 * metagame registers the class for the screen type `remix_create_mode_screen`. The triangle
 * button goes back to the song menu.
 */
class RemixCreateModeScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018b6d0
     * @ghidraAddress PAL: 0x00191e88
     */
    explicit RemixCreateModeScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `remix_create_mode_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c968
     * @ghidraAddress PAL: 0x003caa40
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixCreateModeScreen(pData);
    }

    /**
     * Route a chosen button and the controller buttons, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018b708
     * @ghidraAddress PAL: 0x00191ec0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Mark the remix active unless `modify` was chosen with the cross button, and go to the
     * controller screen.
     *
     * @param pMsg The message.
     * @return True for the cross button, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018b798
     * @ghidraAddress PAL: 0x00191f50
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Go back to the song menu when the triangle button goes down.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x0018b838
     * @ghidraAddress PAL: 0x00191ff0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
