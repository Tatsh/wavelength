#pragma once

#include "msg/joypadinputmsg.h"
#include "os/prnstream.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uistyle.h"

/**
 * Component the player chooses with the cross button.
 *
 * The RTTI records the class as deriving from UIComponent. The object is 0x40 bytes and its vtable
 * is at `0x003d2390`. The panel description provides the name at index 1, the style at index 2,
 * and optionally the base of the object names at index 3, with the name standing in for a missing
 * base. The button shows a mesh `<panel>_<base>.mesh` and a text object `<panel>_<base>.txt`, and
 * its style selects the material of the mesh and the font of the text for each state.
 *
 * Pressing the cross button sends a UIComponentSelectStartMsg. Unless a handler stops the choice,
 * the button then flashes between the selected and the normal state, and sends a
 * UIComponentSelectMsg once the flash ends.
 */
class UIButton : public UIComponent {
public:
    /**
     * Construct a button from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the button belongs to.
     * @ghidraAddress NTSC-U/C: 0x002041c0
     * @ghidraAddress PAL: 0x0020cf78
     */
    UIButton(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the button.
     *
     * @ghidraAddress NTSC-U/C: 0x002043a8
     * @ghidraAddress PAL: 0x0020d160
     */
    ~UIButton() override;

    /**
     * Create a button from its script description.
     *
     * UIManager::Init() registers the routine for the component type `button_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the button belongs to.
     * @return The new button.
     * @ghidraAddress NTSC-U/C: 0x003774c0
     * @ghidraAddress PAL: 0x003e5bf0
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new UIButton(pData, pszPanel);
    }

    /**
     * Handle a controller button, or pass any other message to UIComponent.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00204598
     * @ghidraAddress PAL: 0x0020d350
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show or hide the button, its text, and its mesh.
     *
     * @param bShowing Whether the button shows.
     * @ghidraAddress NTSC-U/C: 0x00204478
     * @ghidraAddress PAL: 0x0020d230
     */
    void SetShowing(bool bShowing) override;

    /**
     * Report the text of the text object.
     *
     * The text object must exist.
     *
     * @return The text as it was last set.
     * @ghidraAddress NTSC-U/C: 0x002043d8
     * @ghidraAddress PAL: 0x0020d190
     */
    const char *Text() const override;

    /**
     * Replace the text of the text object.
     *
     * The text object must exist.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x002043e8
     * @ghidraAddress PAL: 0x0020d1a0
     */
    void SetText(const char *pszText) override;

    /**
     * Change the state and give the mesh and the text the material and the font the style lists
     * for it.
     *
     * @param nState One of UIComponent::State.
     * @param bForce Apply the state even when it does not change.
     * @ghidraAddress NTSC-U/C: 0x002044e8
     * @ghidraAddress PAL: 0x0020d2a0
     */
    void SetState(int nState, bool bForce) override;

    /**
     * Stop a flash that runs and return to the state from before it.
     *
     * @ghidraAddress NTSC-U/C: 0x00204708
     * @ghidraAddress PAL: 0x0020d4c0
     */
    void Unfocus() override;

    /**
     * Advance the flash to a time, and send the UIComponentSelectMsg once it ends.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00204758
     * @ghidraAddress PAL: 0x0020d510
     */
    void Poll(float fTime) override;

    /**
     * Write the name, the visibility, the state, and the flash.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00204938
     * @ghidraAddress PAL: 0x0020d6f0
     */
    void Print(PrnStream &stream) override;

    /**
     * Start a flash between two states.
     *
     * The first selected phase is already over at fTime, and the flash ends after nFlashes periods
     * and a last normal phase. The state from before the flash is restored when it ends.
     *
     * @param fTime The front-end time in milliseconds.
     * @param nFlashes The number of flashes.
     * @param nSelectedState The state of the selected phase.
     * @param nNormalState The state of the normal phase.
     * @param fSelectedMs The length of the selected phase.
     * @param fNormalMs The length of the normal phase.
     * @ghidraAddress NTSC-U/C: 0x00204880
     * @ghidraAddress PAL: 0x0020d638
     */
    virtual void StartFlash(float fTime,
                            int nFlashes,
                            int nSelectedState,
                            int nNormalState,
                            float fSelectedMs,
                            float fNormalMs);

    /**
     * Replace the style and apply the current state again.
     *
     * @param pStyle The style.
     * @ghidraAddress NTSC-U/C: 0x00204418
     * @ghidraAddress PAL: 0x0020d1d0
     */
    void SetStyle(UIStyle *pStyle);

    /**
     * Start the flash when the cross button goes down.
     *
     * @param pMsg The message of the button.
     * @return Whether the button handled the message.
     * @ghidraAddress NTSC-U/C: 0x00204600
     * @ghidraAddress PAL: 0x0020d3b8
     */
    bool HandleSelect(JoypadInputMsg *pMsg);

    bool mFlashDone;    /*!< Whether no flash runs. */
    float mFlashStart;  /*!< The time the first flash period started. */
    float mFlashPeriod; /*!< The length of one selected and one normal phase. */
    float mSelectedMs;  /*!< The length of the selected phase. */
    float mNormalMs;    /*!< The length of the normal phase. */
    float mFlashEnd;    /*!< The time the flash ends. */
    int mNormalState;   /*!< The state of the normal phase. */
    int mSelectedState; /*!< The state of the selected phase. */
    int mRevertState;   /*!< The state to restore when the flash ends. */
    Rnd::Mesh *mMesh;   /*!< The mesh, or null when the file has none of that name. */
    Rnd::Text *mText;   /*!< The text object, or null when the file has none of that name. */
    UIStyle *mStyle;    /*!< The style. */
};
