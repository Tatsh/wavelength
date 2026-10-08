#pragma once

#include <vector>

#include "game/remixinfo.h"
#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * The screen that lists saved remixes to choose one to play or to practise.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003ce718`. The description's `grey_read_only` and `grey_unplayable` grey out the remixes
 * that cannot be changed and the ones that cannot be played, and greyed remixes cannot be chosen.
 * The cross button plays the chosen remix and the square button practises it, both through the
 * `load_remix` screen.
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
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035c418
     * @ghidraAddress PAL: 0x003ca4f0
     */
    ~RemixSelectScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c518
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixSelectScreen(pData);
    }

    /**
     * Route the choice and the controller to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00189de0
     * @ghidraAddress PAL: 0x00190378
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, list the remixes, and select the first.
     *
     * The online screen `net_custom_load` greys out only the remixes that cannot be played, and
     * none in a remix game.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00189938
     * @ghidraAddress PAL: 0x0018fed0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Replace the remixes the screen lists.
     *
     * The name is inferred.
     *
     * @param remixes The remixes.
     * @ghidraAddress NTSC-U/C: 0x0035c558
     * @ghidraAddress PAL: 0x003ca630
     */
    virtual void SetRemixes(const std::vector<RemixInfo> &remixes) {
        mRemixes = remixes;
    }

    std::vector<RemixInfo> mRemixes; /*!< The listed remixes. */
    int mGreyReadOnly;               /*!< Whether remixes that cannot be changed show greyed out. */
    int mGreyUnplayable;             /*!< Whether remixes that cannot be played show greyed out. */

private:
    /**
     * Load the selected remix into GameDb and go to `load_remix`, or to the read-only checks of
     * an online game.
     *
     * Online, the creators of a remix game that can be changed become the players of the game.
     * The name is inferred.
     *
     * @param bPractice Whether the remix is practised rather than played.
     * @return 1 for a greyed remix, 0 after going to a read-only check, and -1 after going to
     * `load_remix`.
     * @ghidraAddress NTSC-U/C: 0x001899f0
     * @ghidraAddress PAL: 0x0018ff88
     */
    int LoadSelectedRemix(bool bPractice);

    /**
     * Play the selected remix with the cross button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of LoadSelectedRemix() unless it went to `load_remix`, otherwise the
     * result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00189e70
     * @ghidraAddress PAL: 0x00190408
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Return to the host screen of an online game with the triangle button, and practise the
     * selected remix with the square button in a solo game.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of UIScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00189ed0
     * @ghidraAddress PAL: 0x00190468
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
