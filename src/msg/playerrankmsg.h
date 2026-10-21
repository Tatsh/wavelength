#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports the ladder position of a player whose position was requested.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d61f8`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class PlayerRankMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039e8c0
     * @ghidraAddress PAL: 0x0040d5a0
     */
    ~PlayerRankMsg() override = default;

    /**
     * Construct a message.
     *
     * Inline.
     *
     * @param nAccount The account of the player.
     * @param nRank The position of the player in the ranking.
     */
    PlayerRankMsg(int nAccount, int nRank) : mAccount(nAccount), mRank(nRank) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "PlayerRankMsg", 0);
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
        return new PlayerRankMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nPlayerRankMsgType.
     */
    int Type() override {
        return g_nPlayerRankMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `PlayerRankMsg`.
     */
    const char *GetName() const override {
        return "PlayerRankMsg";
    }

    /**
     * Write the account and the rank.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268e90
     * @ghidraAddress PAL: 0x00272a98
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return The account of the player. */
    int GetAccount() const {
        return mAccount;
    }

    /** @return The position of the player in the ranking. */
    int GetRank() const {
        return mRank;
    }

private:
    int mAccount; /*!< The account of the player. */
    int mRank;    /*!< The position of the player in the ranking. */
};
