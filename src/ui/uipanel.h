#pragma once

#include <list>
#include <map>

#include "app/msgsink.h"
#include "os/string.h"
#include "os/strless.h"
#include "rnd/animatable.h"
#include "rnd/object.h"
#include "rnd/rndloader.h"
#include "rnd/text.h"
#include "rnd/view.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uinavigator.h"

/**
 * Group of components drawn from one `.rnd` file, which a screen shows.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x90 bytes. UIManager creates
 * one for each `panel` entry of the front-end description, through the factory registered for the
 * entry's type. Index 1 of the entry is the name, and the file is `<directory>/<name>.rnd` beside
 * the description. From index 2 the entry lists the components (each entry type that includes
 * `comp`), the component that first receives the focus (`focus`), the navigator (`navigator`), and
 * the frames of the entry and exit animation (`panel_enter_exit`).
 *
 * Load() and Unload() count references. The first Load() queues the file, and IsLoaded() creates
 * the components once the file has loaded. The file provides the view `<name>.view`, the looping
 * animation `<name>_always.anim`, and the entry and exit animation `<name>_enterexit.anim`. Enter()
 * and Exit() play the entry and exit animation over the time given by its frames, and mState
 * follows them.
 */
class UIPanel : public MsgSink {
public:
    /** Where the panel is in its entry and exit, the values of mState. */
    enum State {
        kStateShown = 0,    /*!< The panel shows and does not animate. */
        kStateHidden = 1,   /*!< The panel has exited, or has not entered yet. */
        kStateEntering = 2, /*!< The entry animation plays. */
        kStateExiting = 3,  /*!< The exit animation plays. */
    };

    /**
     * Construct a hidden panel from its script description.
     *
     * The panel takes a reference to the description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00208758
     * @ghidraAddress PAL: 0x00211558
     */
    UIPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel, its components, and the objects its file loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00208fc0
     * @ghidraAddress PAL: 0x00211dc0
     */
    ~UIPanel() override;

    /**
     * Read the default frames of the entry and exit animation.
     *
     * The `panel_enter_exit` section of the configuration provides `enter_start_frame`,
     * `enter_stop_frame`, `exit_start_frame`, and `exit_stop_frame`. Without the section the
     * defaults remain. A panel's own `panel_enter_exit` entry overrides them.
     *
     * @param pConfig The `ui` section of the configuration.
     * @ghidraAddress NTSC-U/C: 0x002086b0
     * @ghidraAddress PAL: 0x002114b0
     */
    static void Init(DataArray *pConfig);

    /**
     * Create a panel from its script description.
     *
     * UIManager::Init() registers the routine for the entry type `panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00378608
     * @ghidraAddress PAL: 0x003c20c8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new UIPanel(pData, pszDir);
    }

    /**
     * Offer a message to the current screen, the panel, the navigator, and the component with the
     * focus.
     *
     * A message that is neither a controller button nor a keyboard key goes first to the current
     * screen and goes no further than the navigator. A controller button or keyboard key skips the
     * screen and reaches the component with the focus when nothing before handles it.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00209460
     * @ghidraAddress PAL: 0x00212198
     */
    bool Dispatch(Message *pMsg) override;

    /**
     * Handle a message sent to the panel.
     *
     * The routine asks the message for its type and handles nothing.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00209558
     * @ghidraAddress PAL: 0x00212358
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Take a reference to the panel's file, queueing the file on the first reference.
     *
     * The first reference also hides the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00208948
     * @ghidraAddress PAL: 0x00211748
     */
    virtual void Load();

    /**
     * Drop a reference to the panel's file, destroying the components and the loader on the last
     * reference.
     *
     * The name of the component with the focus is kept, and that component receives the focus again
     * after the next load.
     *
     * @ghidraAddress NTSC-U/C: 0x002089b8
     * @ghidraAddress PAL: 0x002117b8
     */
    virtual void Unload();

    /**
     * Start the entry animation.
     *
     * A panel that is exiting reverses into its entry at the matching point of the animation.
     *
     * @param bForce Show the panel at once instead of animating.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00209228
     * @ghidraAddress PAL: 0x00212028
     */
    virtual void Enter(bool bForce, float fTime);

    /**
     * Start the exit animation.
     *
     * A panel that is entering reverses into its exit at the matching point of the animation.
     *
     * @param bForce Hide the panel at once instead of animating.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00209398
     */
    virtual void Exit(bool bForce, float fTime);

    /**
     * React to the screen giving the panel the focus by letting the navigator move the focus.
     *
     * @ghidraAddress NTSC-U/C: 0x002097d8
     * @ghidraAddress PAL: 0x002125d8
     */
    virtual void Focus();

    /**
     * React to the screen taking the focus away.
     *
     * The base routine does nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x00378658
     */
    virtual void Unfocus() {
    }

    /**
     * Draw what goes under the panel.
     *
     * FreqScreen::Draw() calls the routine for each panel before the panels draw. The base routine
     * draws nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x00378660
     */
    virtual void DrawGizmo() {
    }

    /**
     * Advance the panel's animations and its components to a time.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00209658
     * @ghidraAddress PAL: 0x00212458
     */
    virtual void Poll(float fTime);

    /**
     * Draw the panel's view once it has loaded, unless the panel is hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x00209768
     * @ghidraAddress PAL: 0x00212568
     */
    virtual void Draw();

    /**
     * Show or hide the panel's view.
     *
     * @param bShowing Whether the view shows.
     * @ghidraAddress NTSC-U/C: 0x002097a0
     * @ghidraAddress PAL: 0x002125a0
     */
    virtual void SetShowing(bool bShowing);

    /**
     * Move the focus to a component.
     *
     * The panel first dispatches a UIComponentFocusChangeMsg to itself, and a handler that reports
     * it as handled keeps the focus where it is.
     *
     * @param pComponent The component, or null.
     * @param nButton The controller button that moved the focus, or kPadNone. The base routine
     *                ignores it.
     * @ghidraAddress NTSC-U/C: 0x002097f0
     * @ghidraAddress PAL: 0x002125f0
     */
    virtual void SetFocus(UIComponent *pComponent, int nButton);

    /**
     * Find the loaded objects and create the components once the panel's file has loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00208b48
     * @ghidraAddress PAL: 0x00211948
     */
    virtual void FinishLoad();

    /**
     * Report whether the panel's file has loaded, finishing the load when it has.
     *
     * @return Whether the panel is ready to show.
     * @ghidraAddress NTSC-U/C: 0x00208ae8
     * @ghidraAddress PAL: 0x002118e8
     */
    bool IsLoaded();

    /**
     * Register a component under its name.
     *
     * @param pComponent The component.
     * @ghidraAddress NTSC-U/C: 0x002090a0
     * @ghidraAddress PAL: 0x00211ea0
     */
    void AddComponent(UIComponent *pComponent);

    /**
     * Show or hide every text object of the panel's file.
     *
     * @param bShowing Whether the texts show.
     * @ghidraAddress NTSC-U/C: 0x002091a0
     * @ghidraAddress PAL: 0x00211fa0
     */
    void SetTextsShowing(bool bShowing);

    /**
     * Report the frame of the entry and exit animation at a time.
     *
     * An entry or exit that has run its length ends here, which hides the texts of an exit.
     *
     * @param fTime The front-end time in milliseconds.
     * @return The frame, or mIdleFrame while neither animation plays.
     * @ghidraAddress NTSC-U/C: 0x00209588
     * @ghidraAddress PAL: 0x00212388
     */
    float ComputeFrame(float fTime);

    /**
     * Find a component by name.
     *
     * @param pszName The name.
     * @param bFail Treat a missing component as an error. The shipped build ignores it.
     * @return The component, or null.
     * @ghidraAddress NTSC-U/C: 0x002098b8
     * @ghidraAddress PAL: 0x002126b8
     */
    UIComponent *FindComponent(const char *pszName, bool bFail);

    /**
     * Collect and localise the text objects of the panel's file.
     *
     * Each text object is hidden, and its name with any `.txt` removed is the token the locale
     * translates. In the edit mode of the `ui` configuration the routine counts each token's uses
     * and records the tokens the locale lacks for UIManager::ReportLocalizeErrors().
     *
     * @param pszFile The panel's file.
     * @param objects The objects the file loaded.
     * @ghidraAddress NTSC-U/C: 0x0020a278
     * @ghidraAddress PAL: 0x00213078
     */
    void LocalizeTexts(const char *pszFile, std::list<Rnd::Object *> &objects);

    float mFrame;      /*!< The frame of the entry and exit animation the last Poll() computed. */
    float mIdleFrame;  /*!< The frame ComputeFrame() reports while neither animation plays. */
    const char *mName; /*!< The name, a symbol. */
    String mFile;      /*!< The `.rnd` file. */
    int mState;        /*!< One of State. */
    Rnd::View *mView;  /*!< The view of the file, or null. */
    const char *mFocusName;          /*!< The component that receives the focus after a load. */
    float mLoopStart;                /*!< The time the looping animation's frame counts from. */
    Rnd::Animatable *mLoopAnim;      /*!< The looping animation, or null. */
    Rnd::Animatable *mEnterExitAnim; /*!< The entry and exit animation, or null. */
    float mTransitionStart;          /*!< The time the entry or exit started, or 0 for neither. */
    float mShownFrame;               /*!< The frame the entry stops at. */
    float mHiddenFrame;              /*!< The frame the exit stops at. */
    float mEnterMs;                  /*!< The length of the entry. */
    float mExitMs;                   /*!< The length of the exit. */
    bool mEnterReversed;             /*!< Whether the entry plays its frames backwards. */
    bool mExitReversed;              /*!< Whether the exit plays its frames backwards. */
    float mEnterStartFrame;          /*!< The frame the entry starts at. */
    float mExitStartFrame;           /*!< The frame the exit starts at. */
    int mLoadRefs;                 /*!< The number of Load() calls not yet balanced by Unload(). */
    bool mLoaded;                  /*!< Whether FinishLoad() has run since the last load. */
    DataArray *mData;              /*!< The script description. */
    std::list<Rnd::Text *> mTexts; /*!< The text objects of the file. */
    std::map<const char *, UIComponent *, StrLess> mComponents; /*!< The components by name. */
    UINavigator *mNavigator; /*!< The navigator that moves the focus, or null. */
    UIComponent *mFocus;     /*!< The component with the focus, or null. */
    RndLoader *mLoader;      /*!< The loader of the file, or null. */

private:
    /**
     * The default frame the entry starts at.
     *
     * @ghidraAddress NTSC-U/C: 0x003afccc
     */
    static float sEnterStartFrame;

    /**
     * The default frame the entry stops at.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcd0
     */
    static float sEnterStopFrame;

    /**
     * The default frame the exit starts at.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcd4
     */
    static float sExitStartFrame;

    /**
     * The default frame the exit stops at.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcd8
     */
    static float sExitStopFrame;
};
