#pragma once

#include "met/multiendgamescreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The results screen of an online game. The screen ends the game itself after a time.
 *
 * The RTTI records the class as deriving from MultiEndGameScreen. The object is 0xa4 bytes and
 * its vtable is at `0x003cd8c8`. The metagame registers the class for the screen type
 * `fn_end_screen`. The time is `net_endgame_wait_ms` of the `metagame` section of the system
 * configuration.
 */
class NetEndGameScreen : public MultiEndGameScreen {
public:
    /**
     * Construct the screen from its script description, with no time running.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00188810
     * @ghidraAddress PAL: 0x0018ca50
     */
    explicit NetEndGameScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `fn_end_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035bf78
     * @ghidraAddress PAL: 0x003c9ad0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetEndGameScreen(pData);
    }

    /**
     * Route a chosen button and the end of the entry, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00188900
     * @ghidraAddress PAL: 0x0018cb40
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, and end the game once the time has run out.
     *
     * @param fTime The front-end time.
     * @ghidraAddress NTSC-U/C: 0x00188848
     * @ghidraAddress PAL: 0x0018ca88
     */
    void Poll(float fTime) override;

    /**
     * End the game when the cross button chooses a button.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00188990
     * @ghidraAddress PAL: 0x0018cbd0
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Start the time once a screen has entered.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x001889e8
     * @ghidraAddress PAL: 0x0018cc28
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    float mEndTime; /*!< The SystemMs() time at which the game ends, or 0 for none. */
};
