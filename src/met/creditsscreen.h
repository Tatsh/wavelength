#pragma once

#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * A page of the credits. The screen fills its text objects with the next lines of a localised
 * text and moves on after a hold, either to a screen between two pages or to the screen after the
 * credits. The screen between two pages comes back to this one.
 *
 * The RTTI records the class as deriving from UIScreen. The object is 0x58 bytes and its vtable
 * is at `0x003cefc8`. The metagame registers the class for the screen type `credits_screen`.
 * The description provides these:
 *
 * - `text_tag`, the token of the text and the prefix of the text objects `<text_tag>_01.txt`,
 *   `<text_tag>_02.txt`, and so on;
 * - `next_screen`, the screen after the last page;
 * - `between_screen`, the screen shown between two pages;
 * - `hold_time`, how long a page shows, by default one second.
 *
 * Each line of the text ends with a carriage return and a line feed.
 */
class CreditsScreen : public UIScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018c500
     * @ghidraAddress PAL: 0x00193038
     */
    explicit CreditsScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `credits_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d190
     * @ghidraAddress PAL: 0x003cb268
     */
    static UIScreen *New(DataArray *pData) {
        return new CreditsScreen(pData);
    }

    /**
     * Route a controller button, and pass every other message to UIScreen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018c9a8
     * @ghidraAddress PAL: 0x001934e0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, and move on once the hold has run out.
     *
     * @param fTime The front-end time.
     * @ghidraAddress NTSC-U/C: 0x0018c6d8
     * @ghidraAddress PAL: 0x00193210
     */
    void Poll(float fTime) override;

    /**
     * Start the entry and the hold, and show the next page.
     *
     * The text starts over unless the screen comes back from the screen between two pages.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018c5d0
     * @ghidraAddress PAL: 0x00193108
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Exit the credits on the triangle button and move on on any other button.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * UIScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0018c780
     * @ghidraAddress PAL: 0x001932b8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Stop the hold and go to the screen between two pages, or after the last page to the screen
     * after the credits.
     *
     * @ghidraAddress NTSC-U/C: 0x0018c808
     * @ghidraAddress PAL: 0x00193340
     */
    void Advance();

    /**
     * Fill each text object with the next line of the text, or blank it after the last line.
     *
     * @ghidraAddress NTSC-U/C: 0x0018c860
     * @ghidraAddress PAL: 0x00193398
     */
    void ShowPage();

    const char *mTextTag;       /*!< Token of the text and prefix of the text objects. */
    const char *mCursor;        /*!< The next line of the text. */
    const char *mEnd;           /*!< The byte after the terminator of the text. */
    float mHoldMs;              /*!< How long a page shows, in milliseconds. */
    float mEndTime;             /*!< The system time the hold ends at, or -1 while none runs. */
    const char *mBetweenScreen; /*!< The screen shown between two pages. */
    const char *mFinalScreen;   /*!< The screen after the last page. */
};
