#pragma once

#include <vector>

#include "app/msgsink.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uicomponentselectmsg.h"

class UIComponentFocusChangeMsg;
class UIComponentSelectStartMsg;
class UIPanel;

/**
 * Mover of a panel's focus between its components with the directional buttons.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x34 bytes. A panel creates
 * one from the `navigator` array of its description. Each entry of the array is a `horizontal` or
 * a `vertical` list of component names. Left and right move the focus along the horizontal list
 * that holds the focused component, and up and down along the vertical one. A disabled or hidden
 * component is passed over, and the focus wraps at the ends of a list.
 *
 * The navigator also shows the focused component of each list in the selected state and the
 * others in the normal state. It stops moving the focus while a component is being chosen.
 */
class UINavigator : public MsgSink {
public:
    /** The direction of a list, the values FindFocus() takes. */
    enum Axis {
        kAxisHorizontal = 0, /*!< A list left and right move along. */
        kAxisVertical = 1,   /*!< A list up and down move along. */
    };

    /**
     * Construct a navigator from its script description.
     *
     * @param pData The `navigator` array. Each entry from index 1 is a list.
     * @param pPanel The panel whose focus the navigator moves.
     * @ghidraAddress NTSC-U/C: 0x0020ea20
     * @ghidraAddress PAL: 0x00217838
     */
    UINavigator(DataArray *pData, UIPanel *pPanel);

    /**
     * Destroy the navigator.
     *
     * @ghidraAddress NTSC-U/C: 0x0020f3e8
     * @ghidraAddress PAL: 0x00218200
     */
    ~UINavigator() override;

    /**
     * Move the focus for a directional button, and follow the focus and the choices of the panel.
     *
     * @param pMsg The message.
     * @return False, for every message.
     * @ghidraAddress NTSC-U/C: 0x0020f5d0
     * @ghidraAddress PAL: 0x002183e8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Find the focused component in the lists of a direction.
     *
     * The list that holds the component becomes mRow.
     *
     * @param nAxis One of Axis.
     * @return The index of the focused component in its list, or -1 when no list of the direction
     *         holds it.
     * @ghidraAddress NTSC-U/C: 0x0020f6a8
     * @ghidraAddress PAL: 0x002184c0
     */
    int FindFocus(int nAxis);

    /**
     * Move the focus for a pressed directional button.
     *
     * @param pMsg The message of the button.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0020f790
     * @ghidraAddress PAL: 0x002185a8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Show the newly focused component of each list in the selected state.
     *
     * @param pMsg The message of the focus change.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0020f870
     * @ghidraAddress PAL: 0x00218688
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    /**
     * Stop moving the focus while a component is being chosen.
     *
     * @param pMsg The message. The routine ignores it.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0020f968
     * @ghidraAddress PAL: 0x00218780
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Move the focus again once a component has been chosen.
     *
     * @param pMsg The message. The routine ignores it.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0020f978
     * @ghidraAddress PAL: 0x00218790
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Show the focused component of a list in the selected state and the others in the normal
     * state.
     *
     * A disabled component keeps its state.
     *
     * @param pFocus The focused component, or null.
     * @param row The list.
     * @ghidraAddress NTSC-U/C: 0x0020f988
     * @ghidraAddress PAL: 0x002187a0
     */
    void Highlight(UIComponent *pFocus, std::vector<UIComponent *> &row);

    /**
     * Move the focus along mRow to the next component that is enabled and showing.
     *
     * @param bForward Move towards the end of the list rather than towards its start.
     * @param nButton The controller button that moves the focus.
     * @param nIndex The index of the focused component in mRow, or -1 to do nothing.
     * @ghidraAddress NTSC-U/C: 0x0020fa50
     * @ghidraAddress PAL: 0x00218868
     */
    void Step(bool bForward, int nButton, int nIndex);

    UIPanel *mPanel;                                      /*!< The panel whose focus moves. */
    std::vector<std::vector<UIComponent *> > mHorizontal; /*!< The horizontal lists. */
    std::vector<std::vector<UIComponent *> > mVertical;   /*!< The vertical lists. */
    std::vector<UIComponent *> *mRow; /*!< The list the focus last moved along. */
    bool mWrap;                       /*!< Whether the focus wraps at the ends of a list. */
    bool mEnabled;                    /*!< Whether the directional buttons move the focus. */
};
