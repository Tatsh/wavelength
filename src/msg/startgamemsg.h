#pragma once

#include "msg/message.h"
#include "os/mem.h"

/**
 * Identity that StartGameMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b7c
 */
extern int g_nStartGameMsgType;

/**
 * Message that starts the song of an online session on every console.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d5e50`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class StartGameMsg : public Message {
public:
    /**
     * Construct a message with a seed.
     *
     * Inline.
     *
     * @param nSeed The seed of the random numbers every console's logic draws.
     */
    explicit StartGameMsg(int nSeed) : mSeed(nSeed) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "StartGameMsg", 0);
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
     */
    Message *Clone() override {
        return new StartGameMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nStartGameMsgType.
     */
    int Type() override {
        return g_nStartGameMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `StartGameMsg`.
     */
    const char *GetName() const override {
        return "StartGameMsg";
    }

    /**
     * Write the seed.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002690e0
     * @ghidraAddress PAL: 0x00272ce8
     */
    void PrintExtra(PrnStream &stream) const override;

    int mSeed; /*!< The seed of the random numbers every console's logic draws. */
};
