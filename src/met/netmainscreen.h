#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "msg/keyboardkeymsg.h"
#include "msg/lobbyplayersmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreen.h"
#include "ui/uitextentrycompletemsg.h"

/**
 * One of the screens of the online lobby, which share the tabs of the `fn_main` panel and the chat
 * panel `fn_main_c`.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x003ce328`. Moving
 * the focus over a tab of `fn_main` goes to the screen `fn_main_<tab>`. The panels skip their
 * animations between two screens of the class.
 */
class NetMainScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0017c670
     * @ghidraAddress PAL: 0x001802c8
     */
    explicit NetMainScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035a450
     * @ghidraAddress PAL: 0x003c7fa0
     */
    ~NetMainScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035a4f8
     * @ghidraAddress PAL: 0x003c8048
     */
    static UIScreen *New(DataArray *pData) {
        return new NetMainScreen(pData);
    }

    /**
     * Route the tabs, the chat, the controller, and the players of a launchpad to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017c928
     * @ghidraAddress PAL: 0x00180580
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, focus `join` when the panel with the focus has no focused component, and show the
     * help of the focus.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0017c6f8
     * @ghidraAddress PAL: 0x00180350
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Exit, and give the focus back to `fn_main` unless the next screen is of the class.
     *
     * @param pNextScreen The screen that replaces this one, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0017c810
     * @ghidraAddress PAL: 0x00180468
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Report the title, the localised `fn_main_TITLE` with the name of the chat room.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x0017c8e8
     * @ghidraAddress PAL: 0x00180540
     */
    const char *Title() override;

private:
    /**
     * Go to the screen of the tab of `fn_main` that receives the focus.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017ca58
     * @ghidraAddress PAL: 0x001806b0
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Focus the panel of the chosen tab of `fn_main` when the tab belongs to this screen.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect(), or false for a choice outside `fn_main`.
     * @ghidraAddress NTSC-U/C: 0x0017caf8
     * @ghidraAddress PAL: 0x00180750
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Focus the panel of the tab of `fn_main` that the right button chooses.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x0017cc90
     * @ghidraAddress PAL: 0x001808e8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Leave the lobby with the triangle button on `fn_main`, pass the circle button to the chat,
     * and request the data of the `L1_panel` with the L1 button on `fn_main`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0017cda8
     * @ghidraAddress PAL: 0x00180a00
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Pass a key of the keyboard to the chat panel.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return Whether the chat panel handled the key.
     * @ghidraAddress NTSC-U/C: 0x0017cf88
     * @ghidraAddress PAL: 0x00180be0
     */
    bool HandleKeyboardKey(KeyboardKeyMsg *pMsg);

    /**
     * Pass the text typed into the chat entry to the chat panel.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of ChatPanel::HandleTextEntryComplete().
     * @ghidraAddress NTSC-U/C: 0x0017cfe0
     * @ghidraAddress PAL: 0x00180c38
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    /**
     * Show the players of a launchpad on `fn_main_join` while that panel has the focus of the
     * screen `fn_main_join`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017d028
     * @ghidraAddress PAL: 0x00180c80
     */
    bool HandleLobbyPlayers(LobbyPlayersMsg *pMsg);

    String mL1Panel; // The description's `L1_panel`, the panel the L1 button refreshes.
};
