#pragma once

#include <vector>

#include "game/remixinfo.h"
#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The screen that lists saved remixes to choose one.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003ce718`. The metagame registers the class for the screen type `sel_remix_screen`.
 * The description's `grey_read_only` and `grey_unplayable` grey out the remixes that cannot be
 * changed and the ones that cannot be played. The cross button loads the chosen remix, and in a
 * solo game the square button practises it.
 */
class RemixSelectScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no remixes.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001898b0
     * @ghidraAddress PAL: 0x0018fe48
     */
    explicit RemixSelectScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `sel_remix_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c518
     * @ghidraAddress PAL: 0x003ca5f0
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixSelectScreen(pData);
    }

    /**
     * Route a chosen remix and the controller buttons, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00189de0
     * @ghidraAddress PAL: 0x00190378
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry, and list mRemixes with the first selected.
     *
     * The `net_custom_load` screen greys out only the remixes that cannot be played, and none in a
     * remix game.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00189938
     * @ghidraAddress PAL: 0x0018fed0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Replace the listed remixes. Vtable slot 11.
     *
     * @param remixes The remixes.
     * @ghidraAddress NTSC-U/C: 0x0035c558
     * @ghidraAddress PAL: 0x003ca630
     */
    virtual void SetRemixes(const std::vector<RemixInfo> &remixes) {
        mRemixes = remixes;
    }

    /**
     * Load the selected remix, or refuse a greyed-out one with the refusal sound.
     *
     * The routine sets up the game database and `load_remix`, and goes on to the loading, or
     * online to the read-only questions first.
     *
     * @param bPractice Whether the remix is practised.
     * @return 1 for a refused remix, 0 after going to a question, and -1 after going to
     * `load_remix`.
     * @ghidraAddress NTSC-U/C: 0x001899f0
     * @ghidraAddress PAL: 0x0018ff88
     */
    int LoadSelected(bool bPractice);

    /**
     * Load the remix chosen with the cross button.
     *
     * @param pMsg The message.
     * @return The result of LoadSelected() unless that is -1, otherwise the result of
     * UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00189e70
     * @ghidraAddress PAL: 0x00190408
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Go back to the hosting screens online with the first controller's triangle button, or
     * practise the selected remix of a solo game with the square button.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, the result of LoadSelected() unless that is
     * -1, or the result of UIScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00189ed0
     * @ghidraAddress PAL: 0x00190468
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    std::vector<RemixInfo> mRemixes; /*!< The listed remixes. +0x70 */
    int mGreyReadOnly;   /*!< Whether remixes that cannot be changed show greyed out. +0x80 */
    int mGreyUnplayable; /*!< Whether remixes that cannot be played show greyed out. +0x84 */
};
