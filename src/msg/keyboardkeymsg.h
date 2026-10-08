#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"

/**
 * Identity that KeyboardKeyMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2118
 */
extern int g_nKeyboardKeyMsgType;

/**
 * Message the keyboard poll sends when a key of a USB keyboard is pressed.
 *
 * The RTTI includes the class name and records Message as the base. The object is 8 bytes, and its
 * vtable is at `0x003d6990`.
 */
class KeyboardKeyMsg : public Message {
public:
    /**
     * Construct the message of a key.
     *
     * Inline in the keyboard poll.
     *
     * @param nKey The key.
     */
    explicit KeyboardKeyMsg(int nKey) : mKey(nKey) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "KeyboardKeyMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x003a75d0
     */
    Message *Clone() override {
        return new KeyboardKeyMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nKeyboardKeyMsgType.
     * @ghidraAddress NTSC-U/C: 0x003a7620
     */
    int Type() override {
        return g_nKeyboardKeyMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `KeyboardKeyMsg`.
     * @ghidraAddress NTSC-U/C: 0x003a7630
     */
    const char *GetName() const override {
        return "KeyboardKeyMsg";
    }

    /**
     * Write the key.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0028c280
     * @ghidraAddress PAL: 0x00295bd0
     */
    void PrintExtra(PrnStream &stream) const override;

    int mKey; /*!< The key. */
};
