#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/lobbyplayer.h"
#include "netflow/netchatroominfo.h"
#include "os/mem.h"

/**
 * Result of NetLobby::FindPlayer().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class FindPlayerMsg : public Message {
public:
    /**
     * Construct a failure, with a default player and a chatroom whose identifier is -1.
     *
     * Inline. The counts of the chatroom are not set.
     *
     * @param nResult The error.
     */
    explicit FindPlayerMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult 0 when the player was found.
     * @param player The player.
     * @param chatroom The chatroom the player is in.
     */
    FindPlayerMsg(int nResult, const LobbyPlayer &player, const NetChatroomInfo &chatroom)
        : mResult(nResult), mPlayer(player), mChatroom(chatroom) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FindPlayerMsg", 0);
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
        return new FindPlayerMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nFindPlayerMsgType.
     */
    int Type() override {
        return g_nFindPlayerMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `FindPlayerMsg`.
     */
    const char *GetName() const override {
        return "FindPlayerMsg";
    }

    /**
     * Write the error, or the player and the chatroom.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268ed8
     * @ghidraAddress PAL: 0x00272ae0
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return The chatroom the player is in, with an identifier of -1 when the player is offline.
     */
    const NetChatroomInfo &GetChatroom() const {
        return mChatroom;
    }

    /** @return The player. */
    const LobbyPlayer &GetPlayer() const {
        return mPlayer;
    }

    /** @return 0 when the player was found, otherwise a negative error. */
    int GetResult() const {
        return mResult;
    }

private:
    int mResult;               /*!< 0 when the player was found, otherwise a negative error. */
    LobbyPlayer mPlayer;       /*!< The player. */
    NetChatroomInfo mChatroom; /*!< The chatroom the player is in, with an identifier of -1 when the
                                  player is offline. */
};
