#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"

class UIComponent;
class UIPanel;
class UIScreen;

/**
 * Identity that UIComponentSelectMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afcec
 */
extern int g_nUIComponentSelectMsgType;

/**
 * Message a component sends when the player chooses it.
 *
 * The RTTI records the class as deriving from Message. The object is 0x14 bytes. A button sends it
 * once its flash ends, through its own Dispatch(), and the screen that shows the component moves to
 * the screen its transition table lists for the component.
 */
class UIComponentSelectMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. The components expand it on their stack.
     *
     * @param pComponent The chosen component.
     * @param pPanel The panel with the focus.
     * @param pScreen The current screen.
     * @param nButton The controller button that chose the component.
     */
    UIComponentSelectMsg(UIComponent *pComponent, UIPanel *pPanel, UIScreen *pScreen, int nButton)
        : mComponent(pComponent), mPanel(pPanel), mScreen(pScreen), mButton(nButton) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UIComponentSelectMsg", 0);
    }

    /**
     * Release a message.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0037be40
     * @ghidraAddress PAL: 0x003ea570
     */
    Message *Clone() override {
        return new UIComponentSelectMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUIComponentSelectMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037bea8
     * @ghidraAddress PAL: 0x003ea5d8
     */
    int Type() override {
        return g_nUIComponentSelectMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UIComponentSelectMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037beb8
     * @ghidraAddress PAL: 0x003ea5e8
     */
    const char *GetName() const override {
        return "UIComponentSelectMsg";
    }

    /**
     * Write the names of the component, the panel, and the screen.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e600
     * @ghidraAddress PAL: 0x00217418
     */
    void PrintExtra(PrnStream &stream) const override;

    UIComponent *mComponent; /*!< The chosen component. */
    UIPanel *mPanel;         /*!< The panel with the focus. */
    UIScreen *mScreen;       /*!< The current screen. */
    int mButton;             /*!< The controller button that chose the component. */
};
