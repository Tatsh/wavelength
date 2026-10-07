#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "ui/uiscreen.h"

/**
 * Identity that UIScreenChangeMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afcf8
 */
extern int g_nUIScreenChangeMsgType;

/**
 * Message UIManager sends before it leaves the current screen for another.
 *
 * The RTTI records the class as deriving from Message. The object is 0x0c bytes. A sink of the
 * manager that reports the message as handled cancels the change.
 */
class UIScreenChangeMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. UIManager::GotoScreen() expands it on its stack.
     *
     * @param pScreen The screen to change to.
     * @param pOldScreen The current screen.
     */
    UIScreenChangeMsg(UIScreen *pScreen, UIScreen *pOldScreen)
        : mScreen(pScreen), mOldScreen(pOldScreen) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UIScreenChangeMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0037c1d8
     */
    Message *Clone() override {
        return new UIScreenChangeMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUIScreenChangeMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037c230
     */
    int Type() override {
        return g_nUIScreenChangeMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UIScreenChangeMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037c240
     */
    const char *GetName() const override {
        return "UIScreenChangeMsg";
    }

    /**
     * Write the names of both screens.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e870
     * @ghidraAddress PAL: 0x00217688
     */
    void PrintExtra(PrnStream &stream) const override;

    UIScreen *mScreen;    /*!< The screen to change to. */
    UIScreen *mOldScreen; /*!< The current screen. */
};
