#pragma once

#include "game/songentry.h"
#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uipanel.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * The song menu. It lists the songs of the chosen arena and starts the game of a song.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0xc0 bytes and its vtable
 * is at `0x003ceb40`. The metagame registers the class for the screen type `meta_song_screen`, and
 * the front-end description's `s_g_sel_song` screen is one. The `s_g_sel_song_band` panel has the
 * six song buttons `01` to `06`, each with the label `hi_<nn>` of its best score, and the
 * `s_g_sel_song_pic` panel shows the picture, the biography, and the clip of the song with the
 * focus.
 *
 * Exit() waits for the song preview to fade before the exit starts. The destructor at
 * `0x0035dd20` is compiler-generated and has no declaration here.
 */
class MetaSongScreen : public FreqScreen {
public:
    /** The number of song buttons in the `s_g_sel_song_band` panel. */
    static constexpr int kNumSongButtons = 6;

    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00192690
     */
    explicit MetaSongScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `meta_song_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035ddd8
     * @ghidraAddress PAL: 0x003cbeb8
     */
    static UIScreen *New(DataArray *pData) {
        return new MetaSongScreen(pData);
    }

    /**
     * Route a chosen button, a focus change, the end of the entry, and a controller button, and
     * pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00193cd8
     * @ghidraAddress PAL: 0x0019b0e0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, the song preview, and the pending exit.
     *
     * A pending exit starts 100 milliseconds after the preview went idle. Otherwise a chosen clip
     * starts once the picture of its band is placed, and the idle mix of the menu music starts
     * three seconds after the preview went idle with no clip chosen.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00192728
     * @ghidraAddress PAL: 0x00199a48
     */
    void Poll(float fTime) override;

    /**
     * End the song preview and defer the exit until the preview went idle.
     *
     * The menu music returns unless a song was chosen to play.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00194508
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Fill the song buttons of the chosen arena, focus a song, and show the arena scores.
     *
     * The focus rests on the first unlocked song the first player has not finished, or on `01`.
     * Buttons past the last song are disabled and hidden. A solo game outside the `Tutorial`
     * arena shows the arena score and the score to beat.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00193da8
     * @ghidraAddress PAL: 0x0019b1b0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the title of the screen.
     *
     * The tutorial has the title `training_select_TITLE`. Otherwise the localised title receives
     * the mode, the difficulty, and the arena.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x00192a00
     * @ghidraAddress PAL: 0x00199d30
     */
    const char *Title() override;

    /**
     * Report the song of a song button, `0<n>` being the song n of the chosen arena.
     *
     * @param pButton The button.
     * @return The song, or an entry with no data when the button names no song.
     * @ghidraAddress NTSC-U/C: 0x00192ac8
     * @ghidraAddress PAL: 0x00199df8
     */
    SongEntry FindSong(UIComponent *pButton);

    /**
     * Show the details of the song of a button that receives the focus in the
     * `s_g_sel_song_band` panel.
     *
     * The biography, genre, and status texts, the band picture, the preview clip, the help texts,
     * and the score label of the button are updated. A locked song shows why it is locked.
     *
     * @param pButton The button, or null.
     * @param pPanel The panel of the button.
     * @ghidraAddress NTSC-U/C: 0x00192c88
     * @ghidraAddress PAL: 0x00199fb8
     */
    void UpdatePreview(UIComponent *pButton, UIPanel *pPanel);

    /**
     * Show the band picture or the biography of the song.
     *
     * The two labels of the `s_g_sel_song_pic` panel swap to name the view that is not shown.
     *
     * @param bShowPicture Whether the picture shows rather than the biography.
     * @ghidraAddress NTSC-U/C: 0x00193510
     * @ghidraAddress PAL: 0x0019a918
     */
    void ShowPicture(bool bShowPicture);

    /**
     * Fill the button, the score label, and the grade mesh of a song.
     *
     * A song the first player finished in a solo game shows its best score and its grade. A
     * locked song shows why it is locked.
     *
     * @param pSong The song.
     * @param nIndex The index of the song, from 0.
     * @param bLocked Whether the song is locked.
     * @ghidraAddress NTSC-U/C: 0x001937c8
     * @ghidraAddress PAL: 0x0019abd0
     */
    void SetUpSongButton(SongEntry *pSong, int nIndex, bool bLocked);

    /**
     * Start the game of the song chosen with the cross button.
     *
     * A song of type 4 plays the tutorial with no arena. Any other song plays a game. A solo game
     * then shows the tip of a power-up the player unlocked, or changes to
     * `pre_launchpad2launchseq`, or to `pre_solotut2launchseq` for the tutorial. A game of other
     * players changes to `pre_multi2launchseq`. A locked song is not started.
     *
     * @param pMsg The message.
     * @return True while the exit is pending, otherwise the result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00194568
     * @ghidraAddress PAL: 0x0019b910
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Practise the song with the focus with the square button, toggle the picture with the circle
     * button, and go back to the arena menu with the triangle button.
     *
     * Practice is open to a solo game only, and not to a locked song or the tutorial.
     *
     * @param pMsg The message of the button.
     * @return True while the screen changes, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001947a0
     * @ghidraAddress PAL: 0x0019bb38
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Show the details of the song that receives the focus.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00194b00
     * @ghidraAddress PAL: 0x0019bed0
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Show the details of the focused song once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00194b28
     * @ghidraAddress PAL: 0x0019bef8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    String mSong;          /*!< The song chosen to play. */
    String mPreviewSong;   /*!< The song whose clip starts next, or an empty string. */
    float mPreviewMixTime; /*!< The system time the preview mix starts at, or -1. */
    UIPanel *mBandPanel;   /*!< The `s_g_sel_song_band` panel. */
    UIScreen *mExitScreen; /*!< The screen of the deferred exit. */
    float mExitTime;       /*!< The front-end time of the deferred exit. */
    int mExitPending;      /*!< Non-zero while the exit waits for the preview to end. */
    float mExitStart;      /*!< The system time the deferred exit starts at, or 0. */
    int mSongChosen;       /*!< Non-zero when the cross button chose a song to play. */
};
