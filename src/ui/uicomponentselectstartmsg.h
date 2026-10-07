#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

/**
 * Identity that UIComponentSelectStartMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afcf0
 */
extern int g_nUIComponentSelectStartMsgType;

/**
 * Message a component sends when the player presses the button that chooses it.
 *
 * The RTTI records the class as deriving from Message. The object is 0x18 bytes. A button sends it
 * through its own Dispatch() before its flash starts. A handler that reports the message as handled
 * stops the choice.
 */
class UIComponentSelectStartMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. The components expand it on their stack.
     *
     * @param pComponent The component being chosen.
     * @param pPanel The panel with the focus.
     * @param pScreen The current screen.
     * @param nButton The controller button pressed.
     * @param nPad The controller.
     */
    UIComponentSelectStartMsg(
        UIComponent *pComponent, UIPanel *pPanel, UIScreen *pScreen, int nButton, int nPad)
        : mComponent(pComponent), mPanel(pPanel), mScreen(pScreen), mButton(nButton), mPad(nPad) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UIComponentSelectStartMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0037bf70
     */
    Message *Clone() override {
        return new UIComponentSelectStartMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUIComponentSelectStartMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037bfe0
     */
    int Type() override {
        return g_nUIComponentSelectStartMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UIComponentSelectStartMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037bff0
     */
    const char *GetName() const override {
        return "UIComponentSelectStartMsg";
    }

    /**
     * Write the names of the component, the panel, and the screen.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e6c0
     * @ghidraAddress PAL: 0x002174d8
     */
    void PrintExtra(PrnStream &stream) const override;

    UIComponent *mComponent; /*!< The component being chosen. */
    UIPanel *mPanel;         /*!< The panel with the focus. */
    UIScreen *mScreen;       /*!< The current screen. */
    int mButton;             /*!< The controller button pressed. */
    int mPad;                /*!< The controller. */
};
