#pragma once

#include <map>
#include <vector>

#include "app/msgsink.h"
#include "msg/joypadinputmsg.h"
#include "os/prnstream.h"
#include "os/strless.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uipanel.h"

/**
 * One front-end screen, a set of panels shown together with one panel holding the focus.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x3c bytes. UIManager creates
 * one for each `screen` entry of the front-end description, through the factory registered for the
 * entry's type. The entry provides these:
 *
 * - index 1, the name;
 * - `panels`, the names of the panels the screen shows;
 * - `focus`, the panel that first receives the focus, or the first panel by name;
 * - `screen_transitions`, pairs of a component name or a controller button and the screen that
 *   choosing the component or pressing the button leads to;
 * - `force_entry_exit`, whether the panels skip their animations;
 * - `group`, the loading group, which must be one of the `allowable_groups` of the configuration.
 *
 * UIManager::GotoScreen() calls Exit() on the current screen. Once the exit animations have played
 * and the next screen has loaded, Poll() unloads this screen's panels and calls Enter() on the next
 * screen, which becomes current. A screen that has entered dispatches a UITransitionCompleteMsg to
 * itself.
 */
class UIScreen : public MsgSink {
public:
    /** One entry of the screen's transition table. */
    struct Transition {
        const char *mScreen;    /*!< The name of the screen the transition leads to. */
        const char *mComponent; /*!< The component that leads there, or null. */
        int mButton;            /*!< The controller button that leads there, or kPadNone. */
    };

    /**
     * Construct a screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0020a600
     * @ghidraAddress PAL: 0x00213400
     */
    explicit UIScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0020aa10
     * @ghidraAddress PAL: 0x00213810
     */
    ~UIScreen() override;

    /**
     * Create a screen from its script description.
     *
     * UIManager::Init() registers the routine for the entry type `screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00379470
     * @ghidraAddress PAL: 0x003e7ba0
     */
    static UIScreen *New(DataArray *pData) {
        return new UIScreen(pData);
    }

    /**
     * Offer a message to the manager, then handle it here, then pass a navigation message on.
     *
     * A message that is neither a controller button nor a keyboard key goes first to UIManager. A
     * controller button or keyboard key that the screen does not handle goes to the panel with the
     * focus.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0020af00
     * @ghidraAddress PAL: 0x00213d00
     */
    bool Dispatch(Message *pMsg) override;

    /**
     * Follow the transition of a pressed button or a chosen component.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0020afb8
     * @ghidraAddress PAL: 0x00213db8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the panels to a time, and finish the exit or the entry once their animations stop.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0020b138
     * @ghidraAddress PAL: 0x00213f38
     */
    virtual void Poll(float fTime);

    /**
     * Draw the panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0020b3a8
     * @ghidraAddress PAL: 0x002141a8
     */
    virtual void Draw();

    /**
     * Start the exit to another screen.
     *
     * A panel the next screen shares stays, and one of those that is exiting enters again. The
     * others start their exit animations.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0020b6c8
     * @ghidraAddress PAL: 0x002144c8
     */
    virtual void Exit(UIScreen *pNextScreen, float fTime);

    /**
     * Become the current screen and start the entry animations of the panels.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0020b2e0
     * @ghidraAddress PAL: 0x002140e0
     */
    virtual void Enter(UIScreen *pPrevScreen, float fTime);

    /**
     * Write a description of the screen, its panels, and its transitions.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020b888
     * @ghidraAddress PAL: 0x00214688
     */
    virtual void Print(PrnStream &stream);

    /**
     * Empty the transition table.
     *
     * @ghidraAddress NTSC-U/C: 0x0020ab10
     * @ghidraAddress PAL: 0x00213910
     */
    void ClearTransitions();

    /**
     * Append an entry to the transition table.
     *
     * Both names are interned first.
     *
     * @param pszComponent The component that leads to the screen.
     * @param nButton The controller button that leads to the screen.
     * @param pszScreen The name of the screen.
     * @ghidraAddress NTSC-U/C: 0x0020ab50
     * @ghidraAddress PAL: 0x00213950
     */
    void AddTransition(const char *pszComponent, int nButton, const char *pszScreen);

    /**
     * Add a panel to the screen.
     *
     * @param pPanel The panel.
     * @ghidraAddress NTSC-U/C: 0x0020ad90
     * @ghidraAddress PAL: 0x00213b90
     */
    void AddPanel(UIPanel *pPanel);

    /**
     * Follow the transition of a pressed controller button.
     *
     * @param pMsg The message of the button.
     * @return Whether the button is ignored because the screen is changing.
     * @ghidraAddress NTSC-U/C: 0x0020b050
     * @ghidraAddress PAL: 0x00213e50
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Follow the transition of a component chosen with the cross button.
     *
     * @param pMsg The message of the choice.
     * @return Whether a transition was found.
     * @ghidraAddress NTSC-U/C: 0x0020b0c8
     * @ghidraAddress PAL: 0x00213ec8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Move the focus to a panel.
     *
     * @param pPanel The panel, or null.
     * @ghidraAddress NTSC-U/C: 0x0020b420
     * @ghidraAddress PAL: 0x00214220
     */
    void SetFocus(UIPanel *pPanel);

    /**
     * Find a component on any of the screen's panels.
     *
     * A name found on two panels is reported, and the component of the later panel by name wins.
     *
     * @param pszName The name.
     * @return The component, or null.
     * @ghidraAddress NTSC-U/C: 0x0020b490
     * @ghidraAddress PAL: 0x00214290
     */
    UIComponent *FindComponent(const char *pszName);

    /**
     * Find the transition of a component.
     *
     * @param pComponent The component.
     * @param bFail Treat a missing transition as an error. The shipped build ignores it.
     * @return The transition, or null.
     * @ghidraAddress NTSC-U/C: 0x0020b808
     * @ghidraAddress PAL: 0x00214608
     */
    Transition *FindTransition(UIComponent *pComponent, bool bFail);

    /**
     * Find the transition of a controller button.
     *
     * @param nButton One of JoypadButton.
     * @param bFail Treat a missing transition as an error. The shipped build ignores it.
     * @return The transition, or null.
     * @ghidraAddress NTSC-U/C: 0x0020b848
     * @ghidraAddress PAL: 0x00214648
     */
    Transition *FindTransition(int nButton, bool bFail);

    /**
     * Load the panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0020baf0
     * @ghidraAddress PAL: 0x002148f0
     */
    void Load();

    /**
     * Unload the panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0020bb68
     * @ghidraAddress PAL: 0x00214968
     */
    void Unload();

    /**
     * Report whether every panel has loaded.
     *
     * @return Whether the screen is ready to show.
     * @ghidraAddress NTSC-U/C: 0x0020bbe0
     * @ghidraAddress PAL: 0x002149e0
     */
    bool IsLoaded();

    const char *mName;                                  /*!< The name, a symbol. */
    const char *mGroup;                                 /*!< The loading group, a symbol. */
    std::map<const char *, UIPanel *, StrLess> mPanels; /*!< The panels by name. */
    UIPanel *mFocusPanel;                               /*!< The panel with the focus, or null. */
    std::vector<Transition> mTransitions;               /*!< The transition table. */
    UIScreen *mPrevScreen; /*!< The screen this one is entering from, until the entry finishes. */
    UIScreen *mNextScreen; /*!< The screen this one is exiting to, until the exit finishes. */
    bool mForceEntryExit;  /*!< Whether the panels skip their animations. */
};
