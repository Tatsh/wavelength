#pragma once

#include "msg/message.h"
#include "os/mem.h"

/**
 * Identity that PlayerAbortedMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b84
 * @ghidraAddress PAL: 0x0042fa54
 */
extern int g_nPlayerAbortedMsgType;

/**
 * Message that reports a player leaving an online session.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d5dc0`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class PlayerAbortedMsg : public Message {
public:
    /**
     * Construct a message for a player.
     *
     * Inline.
     *
     * @param nNetOrder The player's position in the order of the session.
     */
    explicit PlayerAbortedMsg(int nNetOrder) : mNetOrder(nNetOrder) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "PlayerAbortedMsg", 0);
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
        return new PlayerAbortedMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nPlayerAbortedMsgType.
     */
    int Type() override {
        return g_nPlayerAbortedMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `PlayerAbortedMsg`.
     */
    const char *GetName() const override {
        return "PlayerAbortedMsg";
    }

    /**
     * Write the player.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269190
     * @ghidraAddress PAL: 0x00272d98
     */
    void PrintExtra(PrnStream &stream) const override;

    int mNetOrder; /*!< The player's position in the order of the session. */
};
