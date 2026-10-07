#pragma once

#include "met/setupremixsavescreen.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The remix save screen of one player of a local or online game.
 *
 * The RTTI records the class as deriving from SetupRemixSaveScreen. Its vtables are at
 * `0x003ce618` and `0x003ce5f8`. The metagame registers the class for the screen type
 * `multi_setup_remix_save_screen`. The player is the last digit of the screen's name, such as
 * `m_r_end_remix_2`. Its panel is `m_r_end`.
 */
class MultiSetupRemixSaveScreen : public SetupRemixSaveScreen {
public:
    /**
     * Construct the screen from its script description, for the player its name ends with.
     *
     * A name that does not end with 1 to 4 means the first player.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018ae20
     * @ghidraAddress PAL: 0x001913f0
     */
    explicit MultiSetupRemixSaveScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `multi_setup_remix_save_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c7b8
     * @ghidraAddress PAL: 0x003ca890
     */
    static UIScreen *New(DataArray *pData) {
        return new MultiSetupRemixSaveScreen(pData);
    }

    /**
     * Route the end of the entry, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018b330
     * @ghidraAddress PAL: 0x001919e8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry and show the number of the player.
     *
     * Online, the metagame's flag at `+0x144` is cleared.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018ae98
     * @ghidraAddress PAL: 0x00191468
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Check the name, and show the error of an empty name or of spaces around it, or else save the
     * remix under the name with `save_remix`.
     *
     * In a local game the save leads to the next player's screen, and online back to the
     * launchpad.
     *
     * @ghidraAddress NTSC-U/C: 0x0018af50
     * @ghidraAddress PAL: 0x00191520
     */
    void Proceed() override;

    /**
     * Note a return from `net_share_remix`, and pass the message on.
     *
     * @param pMsg The message.
     * @return The result of SetupRemixSaveScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x0018b398
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    int mFromShare; /*!< Non-zero after a return from `net_share_remix`, until the next save. */
};
