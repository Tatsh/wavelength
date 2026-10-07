#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * Pause menu of a song, which only the controller that paused may use.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003cea20`. The START button resumes the song. The destructor at `0x0035e010` is
 * compiler-generated and has no declaration here.
 */
class PauseScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00195e20
     * @ghidraAddress PAL: 0x0019d1f0
     */
    explicit PauseScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e0a8
     * @ghidraAddress PAL: 0x003cc188
     */
    static UIScreen *New(DataArray *pData) {
        return new PauseScreen(pData);
    }

    /**
     * Route a chosen button and a controller button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00195ee0
     * @ghidraAddress PAL: 0x0019d2b0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Focus `resume` and start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00195e58
     * @ghidraAddress PAL: 0x0019d228
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report no title.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035e0e8
     * @ghidraAddress PAL: 0x003cc1c8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Resume, end, or quit the song for the choice of the cross button.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00195f70
     * @ghidraAddress PAL: 0x0019d340
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Swallow the buttons of other controllers and of an entry or exit, and resume the song when
     * the START button goes down.
     *
     * @param pMsg The message of the button.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00196058
     * @ghidraAddress PAL: 0x0019d428
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mPad; /*!< The controller that paused. */
};
