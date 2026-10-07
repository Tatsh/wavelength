#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"

/**
 * Identity that JoypadInputMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b210c
 */
extern int g_nJoypadInputMsgType;

/**
 * Message the controller poll sends when a button is pressed or released.
 *
 * The RTTI includes the class name and records Message as the base. The object is 0x10 bytes, and
 * its vtable is at `0x003d6a68`.
 */
class JoypadInputMsg : public Message {
public:
    /**
     * Construct the message of a button.
     *
     * Inline. UIManager::Poll() expands it on its stack to repeat a held directional button.
     *
     * @param nPad The controller.
     * @param nButton The button, one of JoypadButton.
     * @param nPressed 1 when the button went down, 0 when it went up.
     */
    JoypadInputMsg(int nPad, int nButton, int nPressed)
        : mPad(nPad), mButton(nButton), mPressed(nPressed) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "JoypadInputMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x003a7240
     */
    Message *Clone() override {
        return new JoypadInputMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nJoypadInputMsgType.
     * @ghidraAddress NTSC-U/C: 0x003a72a0
     */
    int Type() override {
        return g_nJoypadInputMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `JoypadInputMsg`.
     * @ghidraAddress NTSC-U/C: 0x003a72b0
     */
    const char *GetName() const override {
        return "JoypadInputMsg";
    }

    /**
     * Write the controller, the button, and whether it went down.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0028c168
     * @ghidraAddress PAL: 0x00295ab8
     */
    void PrintExtra(PrnStream &stream) const override;

    int mPad;     /*!< The controller. */
    int mButton;  /*!< The button, one of JoypadButton. */
    int mPressed; /*!< 1 when the button went down, 0 when it went up. */
};
