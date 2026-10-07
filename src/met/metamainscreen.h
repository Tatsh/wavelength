#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The main menu, with the solo, multiplayer, online, and options buttons.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003ceea8`. The metagame registers the class for the screen type `meta_main_screen`,
 * and the front-end description's `main` screen is one. Its panels are `main` and `help`, and its
 * transition table leads from `solo_but` to `main2solofreq`, from `multi_but` to
 * `main2multifreq`, from `freqnet_but` to `main2netfreq`, and from `options_but` to `main_opt`.
 * The destructor at `0x0035d410` is compiler-generated and has no declaration here.
 */
class MetaMainScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018d440
     * @ghidraAddress PAL: 0x00193fa8
     */
    explicit MetaMainScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `meta_main_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d4a8
     * @ghidraAddress PAL: 0x003cb580
     */
    static UIScreen *New(DataArray *pData) {
        return new MetaMainScreen(pData);
    }

    /**
     * Route a chosen button and a focus change, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018d510
     * @ghidraAddress PAL: 0x00194408
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Stop the portal sounds and start the exit.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018d4e8
     * @ghidraAddress PAL: 0x001943e0
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Start the entry and, when several controllers were in use, reduce the game to the first
     * player.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018d478
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035d4e8
     * @ghidraAddress PAL: 0x003cb5c0
     */
    const char *Title() override {
        return "";
    }

    /**
     * Set up the game the chosen button of the cross button leads to.
     *
     * Every button resets the skill level to 1, the tutorial, and the practice mode, and sets the
     * community of the game. The solo and online buttons reduce the game to the first player. The
     * multiplayer button first offers to save a Freq the player changed, through the
     * `cur_freq_not_saved` screen.
     *
     * @param pMsg The message.
     * @return True when the save offer opened, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018d5a0
     * @ghidraAddress PAL: 0x00194498
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Play the portal sound of the button that receives the focus in the `main` panel.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0018d8b8
     * @ghidraAddress PAL: 0x001947b0
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);
};
