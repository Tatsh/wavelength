#pragma once

#include "math/quaternion.h"
#include "math/vector3.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Front-end screen of the metagame, which places the menu projector and plays the menu sounds.
 *
 * The RTTI records the class as deriving from UIScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cfcb0`. The metagame registers the class for the screen type `freq_screen`, and
 * nearly every screen class of the front end derives from it. Besides the entries UIScreen reads,
 * the description provides these:
 *
 * - `gizmoOrig`, the position of the projector while the screen shows;
 * - `gizmoRot`, its orientation as three Euler angles;
 * - `no_gizmo`, whether the projector hides;
 * - `no_projector_sfx`, whether entering the screen plays no projector sound;
 * - `needs_transition_sfx`, whether the screen plays the transition sound instead.
 *
 * Only controllers the players use may press buttons. A screen listed in the `shortcuts` section
 * of the metagame configuration chooses the component it names as soon as it has entered.
 */
class FreqScreen : public UIScreen {
public:
    /**
     * Construct a screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0019e3b0
     * @ghidraAddress PAL: 0x001a60c8
     */
    explicit FreqScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0019e490
     * @ghidraAddress PAL: 0x001a61a8
     */
    ~FreqScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `freq_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003609c0
     * @ghidraAddress PAL: 0x003ceed8
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqScreen(pData);
    }

    /**
     * Play the sound of a chosen component, follow a shortcut, and keep unused controllers out.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0019e870
     * @ghidraAddress PAL: 0x001a6588
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Draw what goes under each panel, then the panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0019e4e8
     * @ghidraAddress PAL: 0x001a6200
     */
    void Draw() override;

    /**
     * Move the projector to the next screen's place and start the exit.
     *
     * The projector moves over time in the front end and jumps there at once otherwise, or when
     * this screen hides it.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019e578
     * @ghidraAddress PAL: 0x001a6290
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Play the entry sound, show or hide the projector, and start the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019e698
     * @ghidraAddress PAL: 0x001a63b0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report the localised title of the screen, the token `<name>_TITLE`.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x0019e838
     * @ghidraAddress PAL: 0x001a6550
     */
    virtual const char *Title();

    /**
     * Play the menu sound of a controller button.
     *
     * The cross button and the directional buttons have sounds. Other buttons play nothing.
     *
     * @param nButton One of JoypadButton.
     * @ghidraAddress NTSC-U/C: 0x0019e920
     * @ghidraAddress PAL: 0x001a6638
     */
    virtual void PlayButtonSound(int nButton);

    /**
     * Play the sound of the button that chooses a component.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0019e9a8
     * @ghidraAddress PAL: 0x001a66c0
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Choose the component the `shortcuts` section of the metagame configuration lists for the
     * screen.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0019e9d8
     * @ghidraAddress PAL: 0x001a66f0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Pass on a controller button of a controller the players use, and swallow any other.
     *
     * @param pMsg The message of the button.
     * @return The result of UIScreen::HandleJoypad(), or true for an unused controller.
     * @ghidraAddress NTSC-U/C: 0x0019eaa8
     * @ghidraAddress PAL: 0x001a67c0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    alignas(16) Vector3 mGizmoOrig; /*!< The position of the projector, `gizmoOrig`. */
    alignas(16) Quat mGizmoRot;     /*!< The orientation of the projector, from `gizmoRot`. */
    bool mNoGizmo;                  /*!< Whether the projector hides, `no_gizmo`. */
    bool mNoProjectorSfx;           /*!< Whether entering plays no projector sound. */
    bool mNeedsTransitionSfx;       /*!< Whether entering plays the transition sound. */
};
