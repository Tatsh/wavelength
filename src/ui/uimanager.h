#pragma once

#include <list>
#include <map>

#include "app/msgsink.h"
#include "app/msgsource.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "os/strless.h"
#include "rnd/object.h"
#include "rnd/rndloader.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"
#include "ui/uistyle.h"

/**
 * Owner of every front-end screen, panel, and style, and of the screen that shows.
 *
 * The RTTI records the class as deriving from MsgSource and MsgSink, and includes the class name.
 * The one instance is TheUI. Its MsgSource subobject sits at `+0x00` and its MsgSink subobject at
 * `+0x10`.
 *
 * LoadFile() reads a front-end description and creates its styles, screens, and panels through the
 * factories registered for their entry types. An entry whose type includes `style`, `screen`, or
 * `panel` is one of those, in that order of precedence, and a `focus` entry goes to the screen it
 * names. The metagame registers its screen and panel classes before it loads its description.
 *
 * The manager routes front-end input. A message reaches its sinks until one handles it, then the
 * current screen when the message is a controller button or a keyboard key. Directional buttons
 * held down repeat, first after the longer delay and then after the shorter one.
 */
class UIManager : public MsgSource, public MsgSink {
public:
    /**
     * Callback that keeps a shared `.rnd` file from replacing loaded objects.
     *
     * The RTTI records the class as deriving from RndLoader::Callback. The object is 0x1c bytes.
     * Init() creates one while it loads the shared files of the `shared_rnd_files` list. An object
     * that already exists is skipped, and one missing from `allowed_rnd_merges` is recorded for
     * Report().
     */
    class Callback : public RndLoader::Callback {
    public:
        /**
         * Construct a callback.
         *
         * @param pConfig The `ui` section of the configuration, which must list
         *                `allowed_rnd_merges`.
         * @ghidraAddress NTSC-U/C: 0x0020c9e0
         * @ghidraAddress PAL: 0x002157e0
         */
        explicit Callback(DataArray *pConfig);

        /**
         * Destroy the callback.
         *
         * @ghidraAddress NTSC-U/C: 0x0037b950
         * @ghidraAddress PAL: 0x003ea080
         */
        ~Callback() override {
        }

        /**
         * Create an object only when no object of the name exists.
         *
         * A name that exists and that `allowed_rnd_merges` does not list is recorded.
         *
         * @param pExisting The loaded object with the same name, or null.
         * @param pszName The object name.
         * @param pszClass The class name. The routine ignores it.
         * @return 1 when no object of the name exists, otherwise 0.
         * @ghidraAddress NTSC-U/C: 0x0020ca58
         * @ghidraAddress PAL: 0x00215858
         */
        int ShouldLoad(Rnd::Object *pExisting, const char *pszName, const char *pszClass) override;

        /**
         * Report the objects the loads skipped without permission.
         *
         * @ghidraAddress NTSC-U/C: 0x0020cb00
         * @ghidraAddress PAL: 0x00215900
         */
        void Report();

        DataArray *mAllowed; /*!< The `allowed_rnd_merges` array. */
        String mRejected;    /*!< The names skipped without permission, one per line. */
    };

    /**
     * Routine that creates a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    typedef UIPanel *(*PanelFactory)(DataArray *pData, const char *pszDir);

    /**
     * Routine that creates a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     */
    typedef UIScreen *(*ScreenFactory)(DataArray *pData);

    /**
     * Routine that creates a style from its script description.
     *
     * @param pData The script description.
     * @return The new style.
     */
    typedef UIStyle *(*StyleFactory)(DataArray *pData);

    /**
     * Routine that creates a component from its script description.
     *
     * @param pData The script description.
     * @param pszDir The name of the panel the component belongs to.
     * @return The new component.
     */
    typedef UIComponent *(*ComponentFactory)(DataArray *pData, const char *pszDir);

    /**
     * Construct an empty manager.
     *
     * @ghidraAddress NTSC-U/C: 0x0020c000
     * @ghidraAddress PAL: 0x00214e00
     */
    UIManager();

    /**
     * Destroy the manager.
     *
     * @ghidraAddress NTSC-U/C: 0x0020c3d0
     * @ghidraAddress PAL: 0x002151d0
     */
    ~UIManager() override;

    /**
     * Read the `ui` section of the configuration, load the shared `.rnd` files, and register the
     * built-in panel, screen, style, and component types.
     *
     * @ghidraAddress NTSC-U/C: 0x0020c668
     * @ghidraAddress PAL: 0x00215468
     */
    void Init();

    /**
     * Destroy every style, screen, panel, and shared object.
     *
     * @ghidraAddress NTSC-U/C: 0x0020cb38
     * @ghidraAddress PAL: 0x00215938
     */
    void Terminate();

    /**
     * Register a screen under its name.
     *
     * @param pScreen The screen.
     * @ghidraAddress NTSC-U/C: 0x0020cd90
     * @ghidraAddress PAL: 0x00215b90
     */
    void AddScreen(UIScreen *pScreen);

    /**
     * Register a panel under its name.
     *
     * @param pPanel The panel.
     * @ghidraAddress NTSC-U/C: 0x0020ce90
     * @ghidraAddress PAL: 0x00215c90
     */
    void AddPanel(UIPanel *pPanel);

    /**
     * Register a style under its name.
     *
     * @param pStyle The style.
     * @ghidraAddress NTSC-U/C: 0x0020cf90
     * @ghidraAddress PAL: 0x00215d90
     */
    void AddStyle(UIStyle *pStyle);

    /**
     * Report whether a message is a controller button or a keyboard key.
     *
     * @param pMsg The message.
     * @return Whether the message is a JoypadInputMsg or a KeyboardKeyMsg.
     * @ghidraAddress NTSC-U/C: 0x0020d090
     * @ghidraAddress PAL: 0x00215e90
     */
    bool IsNavigationMsg(Message *pMsg);

    /**
     * Track the directional buttons for the repeat.
     *
     * @param pMsg The message.
     * @return False, for every message.
     * @ghidraAddress NTSC-U/C: 0x0020d110
     * @ghidraAddress PAL: 0x00215f28
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start or stop the repeat of a directional button.
     *
     * Pressing a directional button starts its repeat unless another controller's repeat runs.
     * Releasing the button that repeats stops the repeat.
     *
     * @param pMsg The message of the button.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0020d180
     * @ghidraAddress PAL: 0x00215f98
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Offer a message to the sinks, then to the manager, then to the current screen.
     *
     * Only a controller button or a keyboard key reaches the current screen, and only those skip
     * the sinks.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0020d270
     * @ghidraAddress PAL: 0x00216088
     */
    bool Dispatch(Message *pMsg) override;

    /**
     * Advance the current screen to a time, and send the repeat of a held directional button.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0020d320
     * @ghidraAddress PAL: 0x00216138
     */
    void Poll(float fTime);

    /**
     * Draw the current screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0020d448
     * @ghidraAddress PAL: 0x00216260
     */
    void Draw();

    /**
     * Find a screen by name.
     *
     * @param pszName The name.
     * @param bFail Treat a missing screen as an error. The shipped build ignores it.
     * @return The screen, or null.
     * @ghidraAddress NTSC-U/C: 0x0020d480
     * @ghidraAddress PAL: 0x00216298
     */
    UIScreen *FindScreen(const char *pszName, bool bFail);

    /**
     * Find a panel by name.
     *
     * @param pszName The name.
     * @param bFail Treat a missing panel as an error. The shipped build ignores it.
     * @return The panel, or null.
     * @ghidraAddress NTSC-U/C: 0x0020d540
     * @ghidraAddress PAL: 0x00216358
     */
    UIPanel *FindPanel(const char *pszName, bool bFail);

    /**
     * Find a component of a panel by name.
     *
     * @param pszPanel The name of the panel.
     * @param pszComponent The name of the component.
     * @param bFail Treat a missing panel or component as an error. The shipped build ignores it.
     * @return The component, or null.
     * @ghidraAddress NTSC-U/C: 0x0020d600
     * @ghidraAddress PAL: 0x00216418
     */
    UIComponent *FindComponent(const char *pszPanel, const char *pszComponent, bool bFail);

    /**
     * Find a style by name.
     *
     * @param pszName The name.
     * @param bFail Treat a missing style as an error. The shipped build ignores it.
     * @return The style, or null.
     * @ghidraAddress NTSC-U/C: 0x0020d658
     * @ghidraAddress PAL: 0x00216470
     */
    UIStyle *FindStyle(const char *pszName, bool bFail);

    /**
     * Change to the screen of a name.
     *
     * The routine looks the screen up in TheUI rather than in this manager.
     *
     * @param pszName The name.
     * @ghidraAddress NTSC-U/C: 0x0020d718
     * @ghidraAddress PAL: 0x00216530
     */
    void GotoScreen(const char *pszName);

    /**
     * Change to a screen.
     *
     * Nothing happens when the screen is current or is the one the current screen exits to. The
     * manager otherwise dispatches a UIScreenChangeMsg to itself, and a handler that reports it as
     * handled cancels the change. The new screen loads its panels, and the current screen exits to
     * it, or the new screen enters at once when no screen is current.
     *
     * @param pScreen The screen.
     * @ghidraAddress NTSC-U/C: 0x0020d758
     * @ghidraAddress PAL: 0x00216570
     */
    void GotoScreen(UIScreen *pScreen);

    /**
     * Record the current screen.
     *
     * UIScreen::Enter() calls the routine.
     *
     * @param pScreen The screen.
     * @ghidraAddress NTSC-U/C: 0x0020d848
     * @ghidraAddress PAL: 0x00216660
     */
    void SetCurrentScreen(UIScreen *pScreen);

    /**
     * Report the panel with the focus on the current screen.
     *
     * A screen must be current.
     *
     * @return The panel, or null.
     * @ghidraAddress NTSC-U/C: 0x0020d858
     * @ghidraAddress PAL: 0x00216670
     */
    UIPanel *FocusPanel();

    /**
     * Load the panels of every screen of a loading group.
     *
     * @param pszGroup The group.
     * @ghidraAddress NTSC-U/C: 0x0020d868
     * @ghidraAddress PAL: 0x00216680
     */
    void LoadGroup(const char *pszGroup);

    /**
     * Unload the panels of every screen of a loading group.
     *
     * @param pszGroup The group.
     * @ghidraAddress NTSC-U/C: 0x0020d8f0
     * @ghidraAddress PAL: 0x00216708
     */
    void UnloadGroup(const char *pszGroup);

    /**
     * Report whether every screen of a loading group has loaded.
     *
     * @param pszGroup The group.
     * @return Whether the group is ready to show.
     * @ghidraAddress NTSC-U/C: 0x0020d978
     * @ghidraAddress PAL: 0x00216790
     */
    bool IsGroupLoaded(const char *pszGroup);

    /**
     * Register a panel type.
     *
     * @param pfnFactory The routine that creates a panel of the type.
     * @param pszType The entry type of the front-end description.
     * @ghidraAddress NTSC-U/C: 0x0020da10
     * @ghidraAddress PAL: 0x00216828
     */
    void RegisterPanelType(PanelFactory pfnFactory, const char *pszType);

    /**
     * Register a screen type.
     *
     * @param pfnFactory The routine that creates a screen of the type.
     * @param pszType The entry type of the front-end description.
     * @ghidraAddress NTSC-U/C: 0x0020db28
     * @ghidraAddress PAL: 0x00216940
     */
    void RegisterScreenType(ScreenFactory pfnFactory, const char *pszType);

    /**
     * Register a style type.
     *
     * @param pfnFactory The routine that creates a style of the type.
     * @param pszType The entry type of the front-end description.
     * @ghidraAddress NTSC-U/C: 0x0020dc40
     * @ghidraAddress PAL: 0x00216a58
     */
    void RegisterStyleType(StyleFactory pfnFactory, const char *pszType);

    /**
     * Register a component type.
     *
     * @param pfnFactory The routine that creates a component of the type.
     * @param pszType The entry type of a panel description.
     * @ghidraAddress NTSC-U/C: 0x0020dd58
     * @ghidraAddress PAL: 0x00216b70
     */
    void RegisterComponentType(ComponentFactory pfnFactory, const char *pszType);

    /**
     * Create a panel through the factory registered for its entry type.
     *
     * The type must be registered.
     *
     * @param pData The script description. Index 0 is the type.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x0020de70
     * @ghidraAddress PAL: 0x00216c88
     */
    UIPanel *CreatePanel(DataArray *pData, const char *pszDir);

    /**
     * Create a screen through the factory registered for its entry type.
     *
     * The type must be registered.
     *
     * @param pData The script description. Index 0 is the type.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0020df78
     * @ghidraAddress PAL: 0x00216d90
     */
    UIScreen *CreateScreen(DataArray *pData);

    /**
     * Create a style through the factory registered for its entry type.
     *
     * The type must be registered.
     *
     * @param pData The script description. Index 0 is the type.
     * @return The new style.
     * @ghidraAddress NTSC-U/C: 0x0020e078
     * @ghidraAddress PAL: 0x00216e90
     */
    UIStyle *CreateStyle(DataArray *pData);

    /**
     * Create a component through the factory registered for its entry type.
     *
     * The type must be registered.
     *
     * @param pData The script description. Index 0 is the type.
     * @param pszDir The name of the panel the component belongs to.
     * @return The new component.
     * @ghidraAddress NTSC-U/C: 0x0020e178
     * @ghidraAddress PAL: 0x00216f90
     */
    UIComponent *CreateComponent(DataArray *pData, const char *pszDir);

    /**
     * Read a front-end description and create what it describes.
     *
     * @param pszFile The description file, or null to do nothing.
     * @param bLoadAll Also load every screen at once, waiting for each to finish.
     * @ghidraAddress NTSC-U/C: 0x0020e280
     * @ghidraAddress PAL: 0x00217098
     */
    void LoadFile(const char *pszFile, bool bLoadAll);

    /**
     * Check a loading group against the `allowable_groups` of the configuration.
     *
     * A null group becomes the first allowable group. A group the list does not include is
     * reported.
     *
     * @param ppszGroup The group, which may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0020e4e8
     * @ghidraAddress PAL: 0x00217300
     */
    void VerifyGroup(const char **ppszGroup);

    /**
     * Report the localisation errors the panels recorded while their texts loaded.
     *
     * The body is with LocalizeErrors, the record it reports.
     *
     * @ghidraAddress NTSC-U/C: 0x00209ca0
     * @ghidraAddress PAL: 0x00212aa0
     */
    void ReportLocalizeErrors();

    Callback *mCallback;         /*!< The callback of the shared loads, while Init() runs them. */
    DataArray *mAllowableGroups; /*!< The `allowable_groups` array of the configuration. */
    std::map<const char *, UIScreen *, StrLess> mScreens; /*!< The screens by name. */
    std::map<const char *, UIPanel *, StrLess> mPanels;   /*!< The panels by name. */
    std::map<const char *, UIStyle *, StrLess> mStyles;   /*!< The styles by name. */
    UIScreen *mCurrentScreen;                             /*!< The screen that shows, or null. */
    float mTime;                                          /*!< The time the last Poll() received. */
    bool mEditMode; /*!< Whether the `editmode` setting of the configuration is on. */
    std::list<Rnd::Object *> mSharedObjects;          /*!< The objects the shared files created. */
    std::map<String, PanelFactory> mPanelFactories;   /*!< The panel types. */
    std::map<String, ScreenFactory> mScreenFactories; /*!< The screen types. */
    std::map<String, StyleFactory> mStyleFactories;   /*!< The style types. */
    std::map<String, ComponentFactory> mComponentFactories; /*!< The component types. */
    int mRepeatButton;   /*!< The directional button that repeats. */
    int mRepeatPad;      /*!< The controller whose button repeats, or -1 for none. */
    float mRepeatTime;   /*!< The time of the next repeat, or 0 when no button repeats. */
    bool mRepeatPending; /*!< Whether mRepeatTime still holds a delay rather than a time. */
    int mRepeatCount;    /*!< The number of repeats so far. */

private:
    /**
     * Report the delay before a repeat.
     *
     * @param nCount The number of repeats so far.
     * @return sFirstRepeatDelay for the first repeat, otherwise sRepeatDelay.
     * @ghidraAddress NTSC-U/C: 0x0020d428
     * @ghidraAddress PAL: 0x00216240
     */
    static float RepeatDelay(int nCount);

    /**
     * The delay before the first repeat, the `max_delay_frames` setting of `nav_repeat`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afce4
     */
    static float sFirstRepeatDelay;

    /**
     * The delay between later repeats, the `min_delay_frames` setting of `nav_repeat`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afce8
     */
    static float sRepeatDelay;
};

/**
 * The front-end manager.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b898
 */
extern UIManager TheUI;
