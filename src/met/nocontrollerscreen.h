#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The dialog that prompts the player to plug a controller back in. The cross button of that
 * controller closes it.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x8c bytes and its vtable
 * is at `0x003cef68`. The metagame registers the class for the screen type
 * `no_controller_screen`. The description's `in_game` entry tells whether the dialog interrupts
 * a song or the front end. Closing it resumes the song, or returns to mReturnScreen.
 */
class NoControllerScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018ca10
     * @ghidraAddress PAL: 0x00193548
     */
    explicit NoControllerScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `no_controller_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d278
     * @ghidraAddress PAL: 0x003cb350
     */
    static UIScreen *New(DataArray *pData) {
        return new NoControllerScreen(pData);
    }

    /**
     * Route a controller button, and pass every other message to FreqScreen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018cb58
     * @ghidraAddress PAL: 0x001936b0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry and name the controller port in the dialog text.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018ca98
     * @ghidraAddress PAL: 0x001935d0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Close the dialog when the cross button of mPad goes down.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0018cbc0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mPad;             /*!< The controller to plug back in. */
    int mInGame;          /*!< Whether the dialog interrupts a song. */
    String mReturnScreen; /*!< The front-end screen the dialog returns to. */
};
