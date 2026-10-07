#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

/**
 * Identity that UIComponentFocusChangeMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afcf4
 */
extern int g_nUIComponentFocusChangeMsgType;

/**
 * Message a panel sends before it moves its focus to another component.
 *
 * The RTTI records the class as deriving from Message. The object is 0x14 bytes. A handler that
 * reports the message as handled keeps the focus where it is.
 */
class UIComponentFocusChangeMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. UIPanel::SetFocus() expands it on its stack.
     *
     * @param pComponent The component to receive the focus, or null.
     * @param pOldComponent The component with the focus, or null.
     * @param pPanel The panel.
     * @param pScreen The current screen.
     */
    UIComponentFocusChangeMsg(UIComponent *pComponent,
                              UIComponent *pOldComponent,
                              UIPanel *pPanel,
                              UIScreen *pScreen)
        : mComponent(pComponent), mOldComponent(pOldComponent), mPanel(pPanel), mScreen(pScreen) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UIComponentFocusChangeMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0037c0a0
     * @ghidraAddress PAL: 0x003ea7d0
     */
    Message *Clone() override {
        return new UIComponentFocusChangeMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUIComponentFocusChangeMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037c108
     * @ghidraAddress PAL: 0x003ea838
     */
    int Type() override {
        return g_nUIComponentFocusChangeMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UIComponentFocusChangeMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037c118
     * @ghidraAddress PAL: 0x003ea848
     */
    const char *GetName() const override {
        return "UIComponentFocusChangeMsg";
    }

    /**
     * Write the names of both components, the panel, and the screen.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e780
     * @ghidraAddress PAL: 0x00217598
     */
    void PrintExtra(PrnStream &stream) const override;

    UIComponent *mComponent;    /*!< The component to receive the focus, or null. */
    UIComponent *mOldComponent; /*!< The component with the focus, or null. */
    UIPanel *mPanel;            /*!< The panel. */
    UIScreen *mScreen;          /*!< The current screen. */
};
