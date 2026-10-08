#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"

/**
 * Identity that JoypadConnectionMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2110
 */
extern int g_nJoypadConnectionMsgType;

/**
 * Message the controller poll sends when a controller is connected or disconnected.
 *
 * The RTTI includes the class name and records Message as the base. The object is 0xc bytes, and
 * its vtable is at `0x003d6a20`.
 */
class JoypadConnectionMsg : public Message {
public:
    /**
     * Construct the message of a controller.
     *
     * Inline in the controller poll.
     *
     * @param nPad The controller.
     * @param nConnected 1 when the controller was connected, 0 when it was disconnected.
     */
    JoypadConnectionMsg(int nPad, int nConnected) : mPad(nPad), mConnected(nConnected) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "JoypadConnectionMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x003a7370
     */
    Message *Clone() override {
        return new JoypadConnectionMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nJoypadConnectionMsgType.
     * @ghidraAddress NTSC-U/C: 0x003a73c8
     */
    int Type() override {
        return g_nJoypadConnectionMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `JoypadConnectionMsg`.
     * @ghidraAddress NTSC-U/C: 0x003a73d8
     */
    const char *GetName() const override {
        return "JoypadConnectionMsg";
    }

    /**
     * Write the controller and whether it is connected.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0028c110
     * @ghidraAddress PAL: 0x00295a60
     */
    void PrintExtra(PrnStream &stream) const override;

    int mPad;       /*!< The controller. */
    int mConnected; /*!< 1 when the controller was connected, 0 when it was disconnected. */
};
