#pragma once

#include "met/freqscreen.h"
#include "met/viewanimplayer.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that announces an unlocked boss song, then moves to `boss_journey`.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cf2a8`. The metagame registers the class for the screen type `boss_unlock_screen`.
 * The screen fills the labels `01`, `02`, and `03` of the `d_launch` panel and plays
 * `d_launch.view`. The destructor at `0x0035eec8` is compiler-generated and has no declaration
 * here.
 */
class BossUnlockScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no song.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035ef20
     * @ghidraAddress PAL: 0x003cd000
     */
    explicit BossUnlockScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `boss_unlock_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035ee88
     * @ghidraAddress PAL: 0x003ccf68
     */
    static UIScreen *New(DataArray *pData) {
        return new BossUnlockScreen(pData);
    }

    /**
     * Route the end of a transition, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00198de0
     * @ghidraAddress PAL: 0x001a0300
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Move to `boss_journey` half a second after the view stops.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198c28
     * @ghidraAddress PAL: 0x001a0148
     */
    void Poll(float fTime) override;

    /**
     * Start the exit and stop the view.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198bf8
     * @ghidraAddress PAL: 0x001a0118
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Fill the labels with the announcement and the title of mSong.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198930
     * @ghidraAddress PAL: 0x0019fe50
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Start the journey to the boss arena and the view.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x00198d80
     * @ghidraAddress PAL: 0x001a02a0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    float mGoMs;                /*!< The system time the screen moves on. */
    int mStarted;               /*!< Set while the view plays. */
    const char *mSong;          /*!< The unlocked boss song, a symbol. */
    ViewAnimPlayer mAnimPlayer; /*!< The player of `d_launch.view`. +0x7c */
};
