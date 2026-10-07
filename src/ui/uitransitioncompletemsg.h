#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "ui/uiscreen.h"

/**
 * Identity that UITransitionCompleteMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afcfc
 */
extern int g_nUITransitionCompleteMsgType;

/**
 * Message a screen sends itself once its panels have finished their entry animations.
 *
 * The RTTI records the class as deriving from Message. The object is 0x0c bytes.
 */
class UITransitionCompleteMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. UIScreen::Poll() expands it on its stack.
     *
     * @param pScreen The screen that entered.
     * @param pPrevScreen The screen it entered from, or null.
     */
    UITransitionCompleteMsg(UIScreen *pScreen, UIScreen *pPrevScreen)
        : mScreen(pScreen), mPrevScreen(pPrevScreen) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UITransitionCompleteMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0037c300
     * @ghidraAddress PAL: 0x003eaa30
     */
    Message *Clone() override {
        return new UITransitionCompleteMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUITransitionCompleteMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037c358
     * @ghidraAddress PAL: 0x003eaa88
     */
    int Type() override {
        return g_nUITransitionCompleteMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UITransitionCompleteMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037c368
     * @ghidraAddress PAL: 0x003eaa98
     */
    const char *GetName() const override {
        return "UITransitionCompleteMsg";
    }

    /**
     * Write the names of both screens.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e8f8
     * @ghidraAddress PAL: 0x00217710
     */
    void PrintExtra(PrnStream &stream) const override;

    UIScreen *mScreen;     /*!< The screen that entered. */
    UIScreen *mPrevScreen; /*!< The screen it entered from, or null. */
};
