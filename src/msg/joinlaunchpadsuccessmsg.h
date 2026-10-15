#pragma once

#include "msg/message.h"
#include "netflow/netgameparams.h"
#include "netflow/netlaunchpadplayer.h"
#include "os/mem.h"

/**
 * Identity that JoinLaunchpadSuccessMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b64
 * @ghidraAddress PAL: 0x0042fa34
 */
extern int g_nJoinLaunchpadSuccessMsgType;

/**
 * Message that reports that this console joined an online session.
 *
 * The RTTI records the class as deriving from Message, and its vtable is at `0x003d5fb8`. The
 * members Clone(), Type(), and GetName() are inline.
 */
class JoinLaunchpadSuccessMsg : public Message {
public:
    /**
     * Construct a message with the settings and the player.
     *
     * Inline.
     *
     * @param params The settings of the game the host published.
     * @param player The player of this console in the session.
     */
    JoinLaunchpadSuccessMsg(const NetGameParams &params, const NetLaunchpadPlayer &player)
        : mParams(params), mPlayer(player) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "JoinLaunchpadSuccessMsg", 0);
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
        return new JoinLaunchpadSuccessMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nJoinLaunchpadSuccessMsgType.
     */
    int Type() override {
        return g_nJoinLaunchpadSuccessMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `JoinLaunchpadSuccessMsg`.
     */
    const char *GetName() const override {
        return "JoinLaunchpadSuccessMsg";
    }

    /**
     * Write the settings and the player of this console.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268fc0
     * @ghidraAddress PAL: 0x00272bc8
     */
    void PrintExtra(PrnStream &stream) const override;

    NetGameParams mParams;      /*!< The settings of the game the host published. */
    NetLaunchpadPlayer mPlayer; /*!< The player of this console in the session. */
};
