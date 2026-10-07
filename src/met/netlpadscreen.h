#pragma once

#include <list>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "msg/keyboardkeymsg.h"
#include "netflow/netgameparams.h"
#include "netflow/netlaunchpadplayer.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uitextentrycompletemsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * A launchpad screen of an online session, with the players, the game, and the chat of the
 * session.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x84 bytes and its vtable
 * is at `0x003cd988`. The metagame registers the class for the screen type `lpad_screen`, and the
 * `fn_h_lpad` and `fn_g_lpad` screens of the host and the guests are ones. The description
 * provides `dataPanel`, the panel of the game, and `buttonPanel`, the panel of the commands. The
 * chat panel is `fn_h_lpad_c`. Moving between two launchpad screens skips the animations of the
 * panels.
 */
class NetLpadScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001873a8
     * @ghidraAddress PAL: 0x0018b5e8
     */
    explicit NetLpadScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `lpad_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035bd80
     * @ghidraAddress PAL: 0x003c98d8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetLpadScreen(pData);
    }

    /**
     * Route the messages of the screen, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00187940
     * @ghidraAddress PAL: 0x0018bb80
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, and show the players of the session when they change.
     *
     * Without a session the lost session dialog shows.
     *
     * @param fTime The front-end time.
     * @ghidraAddress NTSC-U/C: 0x00187708
     * @ghidraAddress PAL: 0x0018b948
     */
    void Poll(float fTime) override;

    /**
     * Start the exit, moving the focus to the command panel unless the next screen is another
     * launchpad screen.
     *
     * @param pNextScreen The screen that replaces this one, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00187568
     * @ghidraAddress PAL: 0x0018b7a8
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Start the entry.
     *
     * Coming back from the remix editor, the host tells the guests and the session that the
     * editing is done.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00187468
     * @ghidraAddress PAL: 0x0018b6a8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Take the settings of the game the host published, and show them while this screen is the
     * current one.
     *
     * @param pParams The settings.
     * @ghidraAddress NTSC-U/C: 0x00187640
     * @ghidraAddress PAL: 0x0018b880
     */
    void UpdateGameParams(const NetGameParams *pParams);

    /**
     * Give the circle button to the chat after moving the focus to the command panel, and pass on
     * every button.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00187a70
     * @ghidraAddress PAL: 0x0018bcb0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Give a key of the USB keyboard to the chat.
     *
     * @param pMsg The message.
     * @return Whether the chat handled the key.
     * @ghidraAddress NTSC-U/C: 0x00187b68
     * @ghidraAddress PAL: 0x0018bda8
     */
    bool HandleKeyboardKey(KeyboardKeyMsg *pMsg);

    /**
     * Give typed text to the chat.
     *
     * @param pMsg The message.
     * @return The result of ChatPanel::HandleTextEntryComplete().
     * @ghidraAddress NTSC-U/C: 0x00187bc0
     * @ghidraAddress PAL: 0x0018be00
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    /**
     * Show the screen of the command that receives the focus in the command panel.
     *
     * The `player` command has the screen `<buttonPanel>_play`, and every other command the
     * screen `<buttonPanel>`.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00187c08
     * @ghidraAddress PAL: 0x0018be48
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Move the focus to the game panel when right is pressed on the `player` command.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x00187cc0
     * @ghidraAddress PAL: 0x0018bf00
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Run the command chosen with the cross button.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00187d78
     * @ghidraAddress PAL: 0x0018bfb8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Report to the session that the screen shows, or show the lost session dialog.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00187f58
     * @ghidraAddress PAL: 0x0018c198
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    const char *mDataPanel;                 /*!< The `dataPanel` entry of the description. */
    const char *mButtonPanel;               /*!< The `buttonPanel` entry of the description. */
    std::list<NetLaunchpadPlayer> mPlayers; /*!< The players shown. */
    int mPlayersVersion; /*!< NetLaunchpad::GetPlayersVersion() when mPlayers was taken. */
};
