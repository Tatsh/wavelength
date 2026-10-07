#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * The options menu of the front end.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cc3d0`. Triangle leads back to the main menu, or to the online main menu in an online
 * game. The projector moves aside while the credits show.
 */
class MainOptionsScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00356818
     * @ghidraAddress PAL: 0x003c3a78
     */
    explicit MainOptionsScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00356780
     * @ghidraAddress PAL: 0x003c39e0
     */
    static UIScreen *New(DataArray *pData) {
        return new MainOptionsScreen(pData);
    }

    /**
     * Route a controller button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016b7e8
     * @ghidraAddress PAL: 0x0016e970
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Exit, and move the projector aside when the credits are next.
     *
     * @param pNextScreen The screen that replaces this one.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016b6b8
     * @ghidraAddress PAL: 0x0016e840
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Enter, and move the projector back from the credits.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016b638
     * @ghidraAddress PAL: 0x0016e7c0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Go back to the main menu on Triangle, unless the screen is between screens.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleJoypad(), or true between screens.
     * @ghidraAddress NTSC-U/C: 0x0016b740
     * @ghidraAddress PAL: 0x0016e8c8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
