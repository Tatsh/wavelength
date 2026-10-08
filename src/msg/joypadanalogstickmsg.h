#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"

/**
 * Identity that JoypadAnalogStickMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2114
 */
extern int g_nJoypadAnalogStickMsgType;

/**
 * Message the controller poll sends when an analogue stick moved, while stick messages are on.
 *
 * The RTTI includes the class name and records Message as the base. The object is 0x14 bytes, and
 * its vtable is at `0x003d69d8`.
 */
class JoypadAnalogStickMsg : public Message {
public:
    /**
     * Construct the message of a stick.
     *
     * Inline in the controller poll.
     *
     * @param nPad The controller.
     * @param nStick The stick, 0 for the left one.
     * @param fX The horizontal position, from -1 to 1.
     * @param fY The vertical position, from -1 to 1.
     */
    JoypadAnalogStickMsg(int nPad, int nStick, float fX, float fY)
        : mPad(nPad), mStick(nStick), mX(fX), mY(fY) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "JoypadAnalogStickMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x003a7498
     */
    Message *Clone() override {
        return new JoypadAnalogStickMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nJoypadAnalogStickMsgType.
     * @ghidraAddress NTSC-U/C: 0x003a7500
     */
    int Type() override {
        return g_nJoypadAnalogStickMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `JoypadAnalogStickMsg`.
     * @ghidraAddress NTSC-U/C: 0x003a7510
     */
    const char *GetName() const override {
        return "JoypadAnalogStickMsg";
    }

    /**
     * Write the controller, the stick, and its position.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0028c1e0
     * @ghidraAddress PAL: 0x00295b30
     */
    void PrintExtra(PrnStream &stream) const override;

    int mPad;   /*!< The controller. */
    int mStick; /*!< The stick, 0 for the left one. */
    float mX;   /*!< The horizontal position, from -1 to 1. */
    float mY;   /*!< The vertical position, from -1 to 1. */
};
