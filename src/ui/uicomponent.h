#pragma once

#include "app/msgsink.h"
#include "os/prnstream.h"
#include "script/dataarray.h"

/**
 * One control of a front-end panel, such as a label, a button, or a list.
 *
 * The RTTI records the class as deriving from MsgSink. The class itself is abstract, and no vtable
 * of its own is emitted. A panel creates each component from one entry of its script description
 * through the factory UIManager registered for the entry's type (`label_comp`, `button_comp`,
 * `lrbutton_comp`, `list_comp`, and `text_entry_comp`). Index 1 of the entry is the component's
 * name.
 *
 * A component reports a state that selects the material and the font of its UIStyle. The panel
 * moves the focus between its components with Focus() and Unfocus(). Messages reach a component
 * through Dispatch(), which offers navigation messages to the focused panel of the current screen
 * first.
 */
class UIComponent : public MsgSink {
public:
    /** The state a component shows, the values of mState and the index UIStyle selects with. */
    enum State {
        kStateNormal = 0,   /*!< The component does not have the focus. */
        kStateSelected = 1, /*!< The component has the focus. */
        kStateDisabled = 2, /*!< The component cannot be chosen. */
    };

    /**
     * Construct a showing component in the normal state.
     *
     * Inline. Each derived constructor expands it.
     *
     * @param pData The script description. Index 1 is the name.
     */
    explicit UIComponent(DataArray *pData) : mName(pData->Sym(1)), mShowing(true) {
        mState = kStateNormal;
    }

    /**
     * Read the parameters of the flash a button shows when it is chosen.
     *
     * The `button_select` section of the configuration provides `num_flashes`,
     * `frames_selected`, and `frames_normal`. Without the section the defaults remain.
     *
     * @param pConfig The `ui` section of the configuration.
     * @ghidraAddress NTSC-U/C: 0x00203cb8
     * @ghidraAddress PAL: 0x0020ca70
     */
    static void Init(DataArray *pConfig);

    /**
     * Offer a message to the focused panel of the current screen, then handle it here.
     *
     * A message that is neither a controller button nor a keyboard key goes first to the panel
     * UIManager::FocusPanel() reports. The component handles the message when the panel does not.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00203d58
     * @ghidraAddress PAL: 0x0020cb10
     */
    bool Dispatch(Message *pMsg) override;

    /**
     * Handle a message sent to the component.
     *
     * The base routine asks the message for its type and handles nothing.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00203df0
     * @ghidraAddress PAL: 0x0020cba8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report whether the component is showing.
     *
     * @return mShowing.
     * @ghidraAddress NTSC-U/C: 0x003772b0
     * @ghidraAddress PAL: 0x003e59e0
     */
    virtual bool IsShowing() const {
        return mShowing;
    }

    /**
     * Show or hide the component.
     *
     * @param bShowing Whether the component shows.
     * @ghidraAddress NTSC-U/C: 0x003772b8
     * @ghidraAddress PAL: 0x003e59e8
     */
    virtual void SetShowing(bool bShowing) {
        mShowing = bShowing;
    }

    /**
     * Report the text the component shows.
     *
     * @return An empty string. A component with text overrides the routine.
     * @ghidraAddress NTSC-U/C: 0x00203d40
     * @ghidraAddress PAL: 0x0020caf8
     */
    virtual const char *Text() const;

    /**
     * Replace the text the component shows.
     *
     * The base routine does nothing.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00203d50
     * @ghidraAddress PAL: 0x0020cb08
     */
    virtual void SetText(const char *pszText);

    /**
     * Report the state the component shows.
     *
     * @return One of State.
     * @ghidraAddress NTSC-U/C: 0x003772c0
     * @ghidraAddress PAL: 0x003e59f0
     */
    virtual int GetState() const {
        return mState;
    }

    /**
     * Change the state the component shows.
     *
     * @param nState One of State.
     * @param bForce Apply the state even when it does not change. The base routine ignores it.
     * @ghidraAddress NTSC-U/C: 0x003772c8
     * @ghidraAddress PAL: 0x003e59f8
     */
    virtual void SetState(int nState, [[maybe_unused]] bool bForce) {
        mState = nState;
    }

    /**
     * React to the panel giving the component the focus.
     *
     * The base routine does nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x003772d0
     * @ghidraAddress PAL: 0x003e5a00
     */
    virtual void Focus() {
    }

    /**
     * React to the panel taking the focus away.
     *
     * The base routine does nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x003772d8
     * @ghidraAddress PAL: 0x003e5a08
     */
    virtual void Unfocus() {
    }

    /**
     * Advance the component to a time.
     *
     * The base routine does nothing.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x003772e0
     * @ghidraAddress PAL: 0x003e5a10
     */
    virtual void Poll([[maybe_unused]] float fTime) {
    }

    /**
     * Write a description of the component.
     *
     * @param stream The stream to write to.
     */
    virtual void Print(PrnStream &stream) = 0;

    const char *mName; /*!< The name, a symbol. */
    bool mShowing;     /*!< Whether the component shows. */
    int mState;        /*!< One of State. */

protected:
    /**
     * Number of times a chosen button flashes before it reports the choice.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcb8
     */
    static int sNumFlashes;

    /**
     * Milliseconds of each flash a chosen button spends in the selected state.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcc0
     */
    static float sSelectedMs;

    /**
     * Milliseconds of each flash a chosen button spends in the normal state.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcc4
     */
    static float sNormalMs;
};
