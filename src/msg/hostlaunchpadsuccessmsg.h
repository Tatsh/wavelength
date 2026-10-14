#pragma once

#include "msg/message.h"
#include "netflow/netgameparams.h"
#include "netflow/netlaunchpadplayer.h"
#include "os/mem.h"

/**
 * Identity that HostLaunchpadSuccessMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b60
 */
extern int g_nHostLaunchpadSuccessMsgType;

/**
 * Message that reports that this console now hosts an online session.
 *
 * The RTTI records the class as deriving from Message, and its vtable is at `0x003d6000`. The
 * members Clone(), Type(), and GetName() are inline.
 */
class HostLaunchpadSuccessMsg : public Message {
public:
    /**
     * Construct a message with default settings.
     *
     * Inline.
     *
     * @param player The player of this console in the session.
     */
    explicit HostLaunchpadSuccessMsg(const NetLaunchpadPlayer &player) : mPlayer(player) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "HostLaunchpadSuccessMsg", 0);
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
        return new HostLaunchpadSuccessMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nHostLaunchpadSuccessMsgType.
     */
    int Type() override {
        return g_nHostLaunchpadSuccessMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `HostLaunchpadSuccessMsg`.
     */
    const char *GetName() const override {
        return "HostLaunchpadSuccessMsg";
    }

    /**
     * Write the player of this console.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268f98
     */
    void PrintExtra(PrnStream &stream) const override;

    NetGameParams mParams;      /*!< The settings of the game, left at their defaults. */
    NetLaunchpadPlayer mPlayer; /*!< The player of this console in the session. */
};
