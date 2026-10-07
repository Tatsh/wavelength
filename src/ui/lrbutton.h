#pragma once

#include "msg/joypadinputmsg.h"
#include "rnd/mesh.h"
#include "script/dataarray.h"
#include "ui/uibutton.h"
#include "ui/uistyle.h"

/**
 * Button with a left and a right arrow that the directional buttons press.
 *
 * The RTTI records the class as deriving from UIButton. The object is 0x54 bytes and its vtable is
 * at `0x003d2310`. Beyond the button's own description, index 3 is the base of the object names,
 * which must be present, and index 4 is the style of the arrows. The arrows are the meshes
 * `<panel>_<base>_left.mesh` and `<panel>_<base>_right.mesh`.
 *
 * Pressing left or right on a button with that arrow sends a UIComponentSelectStartMsg, flashes
 * the arrow once, and then sends a UIComponentSelectMsg. The button itself does not flash.
 */
class LRButton : public UIButton {
public:
    /** The arrows, the values SetArrowShowing() takes. */
    enum Arrow {
        kArrowLeft = 0,  /*!< The left arrow. */
        kArrowRight = 1, /*!< The right arrow. */
    };

    /**
     * Construct a button from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the button belongs to.
     * @ghidraAddress NTSC-U/C: 0x00204b98
     * @ghidraAddress PAL: 0x0020d950
     */
    LRButton(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the button.
     *
     * @ghidraAddress NTSC-U/C: 0x00204d48
     * @ghidraAddress PAL: 0x0020db00
     */
    ~LRButton() override;

    /**
     * Create a button from its script description.
     *
     * UIManager::Init() registers the routine for the component type `lrbutton_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the button belongs to.
     * @return The new button.
     * @ghidraAddress NTSC-U/C: 0x00377510
     * @ghidraAddress PAL: 0x003e5c40
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new LRButton(pData, pszPanel);
    }

    /**
     * Handle a controller button, or pass any other message to UIComponent.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00205178
     * @ghidraAddress PAL: 0x0020df30
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show or hide the button and both arrows.
     *
     * @param bShowing Whether the button shows.
     * @ghidraAddress NTSC-U/C: 0x00204ec8
     * @ghidraAddress PAL: 0x0020dc80
     */
    void SetShowing(bool bShowing) override;

    /**
     * Change the state of the button and give both arrows the material of the arrow style.
     *
     * @param nState One of UIComponent::State.
     * @param bForce Apply the state even when it does not change.
     * @ghidraAddress NTSC-U/C: 0x00204dc0
     * @ghidraAddress PAL: 0x0020db78
     */
    void SetState(int nState, bool bForce) override;

    /**
     * Stop a flash that runs and return to the state from before it.
     *
     * @ghidraAddress NTSC-U/C: 0x00204e70
     * @ghidraAddress PAL: 0x0020dc28
     */
    void Unfocus() override;

    /**
     * Advance the flashes of the button and of the pressed arrow to a time.
     *
     * Once the arrow's flash ends, the button sends a UIComponentSelectMsg.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00204f38
     * @ghidraAddress PAL: 0x0020dcf0
     */
    void Poll(float fTime) override;

    /**
     * Start a flash between two states.
     *
     * The override does nothing beyond UIButton::StartFlash().
     *
     * @param fTime The front-end time in milliseconds.
     * @param nFlashes The number of flashes.
     * @param nSelectedState The state of the selected phase.
     * @param nNormalState The state of the normal phase.
     * @param fSelectedMs The length of the selected phase.
     * @param fNormalMs The length of the normal phase.
     * @ghidraAddress NTSC-U/C: 0x00204da0
     * @ghidraAddress PAL: 0x0020db58
     */
    void StartFlash(float fTime,
                    int nFlashes,
                    int nSelectedState,
                    int nNormalState,
                    float fSelectedMs,
                    float fNormalMs) override;

    /**
     * Show or hide one arrow.
     *
     * The arrow must exist.
     *
     * @param nArrow One of Arrow.
     * @param bShowing Whether the arrow shows.
     * @ghidraAddress NTSC-U/C: 0x00205118
     * @ghidraAddress PAL: 0x0020ded0
     */
    void SetArrowShowing(int nArrow, bool bShowing);

    /**
     * Choose the button with the cross button, or press an arrow with left or right.
     *
     * @param pMsg The message of the button.
     * @return Whether the button handled the message.
     * @ghidraAddress NTSC-U/C: 0x002051e0
     * @ghidraAddress PAL: 0x0020df98
     */
    bool HandleSelect(JoypadInputMsg *pMsg);

    Rnd::Mesh *mLeftArrow;  /*!< The left arrow, or null. */
    Rnd::Mesh *mRightArrow; /*!< The right arrow, or null. */
    int mArrowButton;       /*!< The directional button whose arrow flashes, or kPadNone. */
    bool mArrowFlashDone;   /*!< Whether no arrow flashes. */
    UIStyle *mArrowStyle;   /*!< The style of the arrows. */

private:
    /**
     * Number of times a pressed arrow flashes.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcbc
     */
    static int sArrowFlashes;

    /**
     * Give the arrow of mArrowButton the material of a state.
     *
     * Inline. The arrow routines expand it.
     *
     * @param nState One of UIComponent::State.
     * @return Whether mArrowButton has an arrow.
     */
    bool SetArrowState(int nState);
};
