#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uipanel.h"

/**
 * The arena menu. It lists the arenas and the songs of the arena with the focus.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003ceba0`. The metagame registers the class for the screen type `meta_arena_screen`,
 * and the front-end description's `s_g_sel_arena` screen is one. The `s_g_sel_arena` panel has a
 * button for each arena, the first being `Tutorial`, and the `s_g_sel_arena_band` panel lists the
 * songs of the arena with the focus. Choosing an arena waits for the song preview to play before
 * the screen changes. The destructor at `0x0035dc38` is compiler-generated and has no declaration
 * here.
 */
class MetaArenaScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no pending change.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001916e0
     * @ghidraAddress PAL: 0x001989f8
     */
    explicit MetaArenaScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `meta_arena_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035dce0
     * @ghidraAddress PAL: 0x003cbdb8
     */
    static UIScreen *New(DataArray *pData) {
        return new MetaArenaScreen(pData);
    }

    /**
     * Route a chosen button, a focus change, and a controller button, and pass on every other
     * message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00191750
     * @ghidraAddress PAL: 0x00198a68
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, and change to mPendingScreen once the song preview plays.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001924e8
     * @ghidraAddress PAL: 0x00199800
     */
    void Poll(float fTime) override;

    /**
     * Enable the arenas that have songs, grey out the locked ones, and focus an arena, then start
     * the entry.
     *
     * A game of other players disables the `Tutorial` arena, and offers the first arena when none
     * is unlocked. The focus returns to mArena when the song screen leads back here, and rests on
     * the last unlocked arena otherwise.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00191800
     * @ghidraAddress PAL: 0x00198b18
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the title of the screen, the localised title with the mode and the difficulty.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x00192550
     * @ghidraAddress PAL: 0x00199868
     */
    const char *Title() override;

    /**
     * List the songs of an arena that receives the focus in the `s_g_sel_arena` panel.
     *
     * The `s_g_sel_arena_band` panel labels each song with its artist, or with the reason it is
     * locked. A solo game outside the `Tutorial` arena also shows the arena score and the score
     * to beat.
     *
     * @param pArena The arena button, or null.
     * @param pPanel The panel of the button.
     * @ghidraAddress NTSC-U/C: 0x00191ca8
     * @ghidraAddress PAL: 0x00198fc0
     */
    void UpdateBand(UIComponent *pArena, UIPanel *pPanel);

    /**
     * List the songs of the arena that receives the focus, and play the wheel sound when the
     * focus moves to another arena than `Tutorial`.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001922d0
     * @ghidraAddress PAL: 0x001995e8
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Choose the arena of a button that is not greyed out with the cross button.
     *
     * The sound output is silenced and the arena recorded in Metagame::mArena. A solo game sets
     * the tutorial for the `Tutorial` arena and continues to `soloarena2solotut`, and to
     * `soloarena2solosong` for any other arena. A game of other players continues to
     * `multiarena2multisong`, and cannot choose `Tutorial`.
     *
     * @param pMsg The message.
     * @return True while a change is pending, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00192360
     * @ghidraAddress PAL: 0x00199678
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Swallow buttons while the screen changes, and go back to the skill menu with the triangle
     * button.
     *
     * A solo game goes back to `soloarena2soloskill`, a remix to `m_r_mode`, and any other game
     * to `multiarena2multiskill`, and Metagame::mArena is cleared.
     *
     * @param pMsg The message of the button.
     * @return True while the screen changes, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x001925b0
     * @ghidraAddress PAL: 0x001998c8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    UIComponent *mFocusArena; /*!< The arena button that last had the focus, or null. */
    String mPendingScreen;    /*!< The screen to change to once the change is pending. */
    int mChangePending;       /*!< Non-zero while the screen waits to change to mPendingScreen. */
};
