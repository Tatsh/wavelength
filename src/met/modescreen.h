#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The mode menu of a solo or local game: a game, a remix, a duel, the jukebox, or the controller
 * settings.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cecc0`. The metagame registers the class for the screen type `mode_screen`, and the
 * front-end description's `s_mode` and `m_mode` screens are ones. The solo screen's transition
 * table leads from `game_but` to `s_g_sel_skill`, from `remix_but` to `s_r_mode`, from
 * `jbox_but` to `jbox_redbook`, and from `control_but` to `control_config`. The destructor at
 * `0x0035d930` is compiler-generated and has no declaration here.
 */
class ModeScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018de80
     * @ghidraAddress PAL: 0x00194de8
     */
    explicit ModeScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `mode_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d9c8
     * @ghidraAddress PAL: 0x003cbaa0
     */
    static UIScreen *New(DataArray *pData) {
        return new ModeScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018deb8
     * @ghidraAddress PAL: 0x00194e20
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry, hide the avatar of a local game, and disable the jukebox of a solo game
     * until the player finished a song.
     *
     * The jukebox stays enabled while the jukebox screen lists every song.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018df20
     * @ghidraAddress PAL: 0x00194e88
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set the rule set the button chosen with the cross button leads to.
     *
     * The game button selects a game, the duel button a duel between the first player and a
     * second player at skill level 1, and any other button a remix. Each clears the tutorial.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018e140
     * @ghidraAddress PAL: 0x001950a8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
