#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "rnd/view.h"
#include "script/dataarray.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The title screen, which waits for a player to press a button and otherwise starts the attract
 * demo.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003cef08`. The metagame registers the class for the screen type `meta_start_screen`,
 * and the front-end description's `start` screen is one. Its panels are `logo` and `logo_start`,
 * and its transition table leads to the main menu.
 *
 * Once the screen has entered, the attract time is set from the `attract_delay_ms` setting of the
 * metagame configuration, or from `attract_short_delay_ms` after a demo. When the system clock
 * passes it, the next song of `attract_songs` starts as a demo. The destructor at `0x0035d328` is
 * compiler-generated and has no declaration here.
 */
class MetaStartScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no attract time.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018cc68
     * @ghidraAddress PAL: 0x001937d0
     */
    explicit MetaStartScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `meta_start_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d3c0
     * @ghidraAddress PAL: 0x003cb498
     */
    static UIScreen *New(DataArray *pData) {
        return new MetaStartScreen(pData);
    }

    /**
     * Arm the attract demo once the screen has entered, and pass on the controller buttons.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018d3b0
     * @ghidraAddress PAL: 0x00193f18
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, follow the music with the logo animation, and start the attract demo
     * when its time has passed.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018ccb0
     * @ghidraAddress PAL: 0x00193818
     */
    void Poll(float fTime) override;

    /**
     * Disarm the attract demo and start the exit.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018ceb8
     * @ghidraAddress PAL: 0x00193a20
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Blank the version text, find the logo animation, and start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018cd70
     * @ghidraAddress PAL: 0x001938d8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the title of the screen, which has none.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035d400
     * @ghidraAddress PAL: 0x003cb4d8
     */
    const char *Title() override {
        return "";
    }

    /**
     * Start the next song of `attract_songs` as a demo, when `attract_enabled` is set.
     *
     * The players are the first player with their own profile, or a player with the default name,
     * and default players up to the song's `num_players`. The screen then changes to
     * `pre_launchpad2launchseq`.
     *
     * @ghidraAddress NTSC-U/C: 0x0018cee8
     * @ghidraAddress PAL: 0x00193a50
     */
    void LaunchAttract();

    /**
     * Set the attract time once the screen has entered, then follow a shortcut.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleTransitionComplete().
     * @ghidraAddress NTSC-U/C: 0x0018d200
     * @ghidraAddress PAL: 0x00193d68
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Play the choice sound for the cross or START button of the first controller, then pass the
     * button on.
     *
     * A button is swallowed while the screen changes.
     *
     * @param pMsg The message of the button.
     * @return Whether the button was handled.
     * @ghidraAddress NTSC-U/C: 0x0018d328
     * @ghidraAddress PAL: 0x00193e90
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    float mAttractTime;    /*!< The system time the attract demo starts at, or -1 when disarmed. */
    Rnd::View *mMusicView; /*!< The `logo_music_anim.view` the music animates, or null. */
};
