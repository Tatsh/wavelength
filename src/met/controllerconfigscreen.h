#pragma once

#include <vector>

#include "game/inputmap.h"
#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The controller menu, which assigns each game event to a button or a stick.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0xb0 bytes and its vtable
 * is at `0x003cc310`. The `controller_config` entry of the metagame configuration lists the events
 * in `button_events` and `stick_events` and, in `choice_lists`, the buttons or sticks each event
 * may take. Each event entry gives its name, the InputMap action it binds, and the name of its
 * choice list. Each component of the `o_controller` panel is named after an event and shows the
 * button or stick chosen for it, as a column of its choice list.
 */
class ControllerConfigScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0016c0a8
     * @ghidraAddress PAL: 0x0016f230
     */
    explicit ControllerConfigScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003569c0
     * @ghidraAddress PAL: 0x003c3c20
     */
    static UIScreen *New(DataArray *pData) {
        return new ControllerConfigScreen(pData);
    }

    /**
     * Route the start of a choice, a focus change, and a controller button, and pass on every
     * other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016d6d8
     * @ghidraAddress PAL: 0x00170900
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Exit, and stop showing the controller map.
     *
     * @param pNextScreen The screen that replaces this one.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016c2f0
     * @ghidraAddress PAL: 0x0016f4c8
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Enter, remember the screen to return to, read the event lists, and show the first player's
     * bindings.
     *
     * @param pPrevScreen The screen this one replaces, and the one `save` returns to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016c100
     * @ghidraAddress PAL: 0x0016f288
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Show the bindings of an input map.
     *
     * @param pMap The input map.
     * @ghidraAddress NTSC-U/C: 0x0016c310
     * @ghidraAddress PAL: 0x0016f4e8
     */
    void LoadBindings(InputMap *pMap);

    /**
     * Store the bindings the menu shows in the input map of the first player.
     *
     * Every button a choice list names is cleared first.
     *
     * @ghidraAddress NTSC-U/C: 0x0016c488
     * @ghidraAddress PAL: 0x0016f660
     */
    void SaveBindings();

    /**
     * Find the button event that binds an action and may take a button.
     *
     * @param nAction The InputMap action.
     * @param nButton The button, one of JoypadButton.
     * @return The event, or null.
     * @ghidraAddress NTSC-U/C: 0x0016c690
     * @ghidraAddress PAL: 0x0016f868
     */
    DataArray *FindButtonEvent(int nAction, int nButton);

    /**
     * Find the stick event that binds an action.
     *
     * @param nAction The InputMap action.
     * @param nStick The stick. The body does not read it.
     * @return The event, or null.
     * @ghidraAddress NTSC-U/C: 0x0016c798
     * @ghidraAddress PAL: 0x0016f970
     */
    DataArray *FindStickEvent(int nAction, int nStick);

    /**
     * Find an event by name among the button events, then among the stick events.
     *
     * The names are symbols and are compared as pointers.
     *
     * @param pszName The name.
     * @param pnButton Receives 1 for a button event, or 0 for a stick event.
     * @return The event, or null with a warning.
     * @ghidraAddress NTSC-U/C: 0x0016c828
     * @ghidraAddress PAL: 0x0016fa00
     */
    DataArray *FindEvent(const char *pszName, int *pnButton);

    /**
     * Report the label of a button.
     *
     * @param nButton The button, one of JoypadButton.
     * @return The letter of the button's icon, or an empty string past the shoulder buttons and
     * the face buttons.
     * @ghidraAddress NTSC-U/C: 0x0016c948
     * @ghidraAddress PAL: 0x0016fb20
     */
    const char *ButtonLabel(int nButton) const;

    /**
     * Report the label of a stick.
     *
     * @param nStick 0 for the left stick or 1 for the right stick.
     * @return The localized label, or an empty string for another value.
     * @ghidraAddress NTSC-U/C: 0x0016c9d8
     * @ghidraAddress PAL: 0x0016fbb0
     */
    const char *StickLabel(int nStick) const;

    /**
     * Show a label on the component of an event in the style of a button or of a stick.
     *
     * @param pszComponent The component, named after the event.
     * @param pszLabel The label.
     * @param bButton Whether the label is a button's.
     * @ghidraAddress NTSC-U/C: 0x0016ca38
     * @ghidraAddress PAL: 0x0016fc10
     */
    void ApplyLabel(const char *pszComponent, const char *pszLabel, bool bButton);

    /**
     * Report the slot of an event in its list.
     *
     * @param pEvent The event.
     * @return The index of the event after the list's name, or 0 with a warning.
     * @ghidraAddress NTSC-U/C: 0x0016cb40
     * @ghidraAddress PAL: 0x0016fd18
     */
    int EventSlot(DataArray *pEvent);

    /**
     * Report the name of an event.
     *
     * @param pEvent The event.
     * @return The name, a symbol.
     * @ghidraAddress NTSC-U/C: 0x0016cc30
     * @ghidraAddress PAL: 0x0016fe08
     */
    const char *EventName(DataArray *pEvent) const;

    /**
     * Report the InputMap action an event binds.
     *
     * @param pEvent The event.
     * @return The action.
     * @ghidraAddress NTSC-U/C: 0x0016cc50
     * @ghidraAddress PAL: 0x0016fe28
     */
    int EventAction(DataArray *pEvent) const;

    /**
     * Report the choice list of an event.
     *
     * @param pEvent The event.
     * @return The choice list.
     * @ghidraAddress NTSC-U/C: 0x0016cc70
     * @ghidraAddress PAL: 0x0016fe48
     */
    DataArray *ChoiceList(DataArray *pEvent);

    /**
     * Report the column of the choice list of an event that holds a button or a stick.
     *
     * @param pEvent The event.
     * @param nValue The button or the stick.
     * @return The column, or 0 with a warning.
     * @ghidraAddress NTSC-U/C: 0x0016ccb0
     * @ghidraAddress PAL: 0x0016fe88
     */
    int ChoiceColumn(DataArray *pEvent, int nValue);

    /**
     * Mark every other event of the kind of the focused event that takes the same choice, and
     * report whether every event has a choice.
     *
     * @return Whether no component shows the mark `?`.
     * @ghidraAddress NTSC-U/C: 0x0016cd80
     * @ghidraAddress PAL: 0x0016ff58
     */
    bool Validate();

    /**
     * Show the default bindings.
     *
     * @ghidraAddress NTSC-U/C: 0x0016d028
     * @ghidraAddress PAL: 0x00170200
     */
    void ResetBindings();

    /**
     * Step the choice of an event back or forward with the directional buttons.
     *
     * The choices start at column 2 of the choice list and wrap at either end.
     *
     * @param pszComponent The component of the event.
     * @param nButton The button pressed, one of JoypadButton.
     * @ghidraAddress NTSC-U/C: 0x0016d070
     * @ghidraAddress PAL: 0x00170248
     */
    void CycleChoice(const char *pszComponent, int nButton);

    /**
     * Highlight the button or the stick of an event on the controller map.
     *
     * @param pszComponent The component of the event.
     * @ghidraAddress NTSC-U/C: 0x0016d1b8
     * @ghidraAddress PAL: 0x00170390
     */
    void ShowOnMap(const char *pszComponent);

    /**
     * Restore the defaults on `default`, store the bindings and leave on `save`, and step the
     * choice of an event with the directional buttons.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x0016d368
     * @ghidraAddress PAL: 0x00170540
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Check the choices when the focus moves up or down, and go back on Triangle, unless the
     * screen is between screens.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleJoypad(), or true between screens.
     * @ghidraAddress NTSC-U/C: 0x0016d578
     * @ghidraAddress PAL: 0x00170750
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Highlight the event that gained the focus on the controller map and show its hint.
     *
     * @param pMsg The message.
     * @return false.
     * @ghidraAddress NTSC-U/C: 0x0016d618
     * @ghidraAddress PAL: 0x00170840
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    UIScreen *mReturnScreen;         /*!< The screen the menu was entered from, or null. */
    const char *mFocusName;          /*!< The component of the event the focus is on. */
    int mActive;                     /*!< Whether the screen shows, between Enter() and Exit(). */
    DataArray *mChoiceLists;         /*!< The `choice_lists` entry. */
    DataArray *mButtonEvents;        /*!< The `button_events` entry. */
    DataArray *mStickEvents;         /*!< The `stick_events` entry. */
    std::vector<int> mButtonChoices; /*!< The chosen column of each button event. */
    std::vector<int> mStickChoices;  /*!< The chosen column of each stick event. */
};
