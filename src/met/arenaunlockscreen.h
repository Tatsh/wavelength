#pragma once

#include "met/freqscreen.h"
#include "met/viewanimplayer.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that announces the arena unlocked last, then moves to the animation of that arena.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cf188`. The metagame registers the class for the screen type
 * `arena_unlock_screen`. The screen fills the label `03` of the `s_unlock_arena` panel and plays
 * `s_unlock_arena.view`. The description's `hold_time` is how long the screen holds after the
 * view. The destructor at `0x0035f218` is compiler-generated and has no declaration here.
 */
class ArenaUnlockScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00198fb8
     * @ghidraAddress PAL: 0x001a04d8
     */
    explicit ArenaUnlockScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `arena_unlock_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035f1d8
     * @ghidraAddress PAL: 0x003cd2b8
     */
    static UIScreen *New(DataArray *pData) {
        return new ArenaUnlockScreen(pData);
    }

    /**
     * Route the end of a transition, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00199378
     * @ghidraAddress PAL: 0x001a0898
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Move to `unlockarena2anim_0<count>` once the view has stopped and mHoldMs has passed.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00199250
     * @ghidraAddress PAL: 0x001a0770
     */
    void Poll(float fTime) override;

    /**
     * Start the exit and stop the view.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00199220
     * @ghidraAddress PAL: 0x001a0740
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Count the unlocked arenas and name the last in the label.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00199028
     * @ghidraAddress PAL: 0x001a0548
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Start the view.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x00199328
     * @ghidraAddress PAL: 0x001a0848
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    float mStartMs;             /*!< The front-end time the view started, or 0 once it stopped. */
    ViewAnimPlayer mAnimPlayer; /*!< The player of `s_unlock_arena.view`. +0x74 */
    int mCount;                 /*!< The number of unlocked arenas. +0x84 */
    float mHoldMs;              /*!< `hold_time`, the time the screen holds after the view. */
    float mGoMs;                /*!< The front-end time the screen moves on, or 0 for none. */
};
