#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"

/**
 * One page of the tips, which the cross button turns forward and the triangle button back.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003ce8a0`. The description provides `page`, `num_pages`, `next_screen`, and
 * `prev_screen`, and the label `page` of the `tips_help` panel shows the page number. The
 * destructor at `0x0035e440` is compiler-generated and has no declaration here.
 */
class TipsScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00196198
     * @ghidraAddress PAL: 0x0019d568
     */
    explicit TipsScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035e400
     * @ghidraAddress PAL: 0x003cc4e0
     */
    static UIScreen *New(DataArray *pData) {
        return new TipsScreen(pData);
    }

    /**
     * Route a controller button to HandleJoypad(), and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001963e8
     * @ghidraAddress PAL: 0x0019d7b8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the page number and start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00196250
     * @ghidraAddress PAL: 0x0019d620
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Turn the page with a button that FreqScreen::HandleJoypad() passes on.
     *
     * @param pMsg The message of the button.
     * @return The result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00196358
     * @ghidraAddress PAL: 0x0019d728
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mNumPages;               /*!< `num_pages`, or -1. */
    int mPage;                   /*!< `page`, the number of this page, or -1. */
    const char *mNextScreenName; /*!< `next_screen`, the next page, or null. */
    const char *mPrevScreenName; /*!< `prev_screen`, the previous page, or null. */
};
