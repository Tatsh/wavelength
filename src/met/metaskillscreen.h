#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The skill menu of a game, with a button for each skill level, the custom remixes, and the
 * power-up tips.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cec60`. The metagame registers the class for the screen type `meta_skill_screen`,
 * and the front-end description's `s_g_sel_skill` screen is one. The destructor at `0x0035da08` is
 * compiler-generated and has no declaration here.
 */
class MetaSkillScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001908a0
     * @ghidraAddress PAL: 0x00197bb8
     */
    explicit MetaSkillScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `meta_skill_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035daa0
     * @ghidraAddress PAL: 0x003cbb78
     */
    static UIScreen *New(DataArray *pData) {
        return new MetaSkillScreen(pData);
    }

    /**
     * Route a chosen button, a focus change, and the end of the entry, and pass on every other
     * message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001908d8
     * @ghidraAddress PAL: 0x00197bf0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Focus the button of the current skill level, then start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00190ca8
     * @ghidraAddress PAL: 0x00197fc0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set the skill level of the button chosen with the cross button and go to the next screen.
     *
     * The tutorial and the saved remix are cleared first. The custom button loads a saved remix
     * instead. A solo game continues to `soloskill2soloarena`, a duel to `duel2multisong`, and
     * any other game to `m_powerup`. The unlocked arenas are shown before the change.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00190988
     * @ghidraAddress PAL: 0x00197ca0
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Show the help text `<panel>_<button>_HELP` of the button that receives the focus in the
     * panel with the focus.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00190bb0
     * @ghidraAddress PAL: 0x00197ec8
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Focus the focused button of the panel with the focus again once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00190c60
     * @ghidraAddress PAL: 0x00197f78
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
