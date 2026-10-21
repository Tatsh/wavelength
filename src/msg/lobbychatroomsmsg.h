#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netchatroominfo.h"
#include "os/mem.h"

/**
 * Message that delivers the chat rooms of the lobby.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6168`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class LobbyChatroomsMsg : public Message {
public:
    /**
     * Construct a message that points at the rooms.
     *
     * Inline.
     *
     * @param pChatrooms The rooms. The caller retains them.
     */
    explicit LobbyChatroomsMsg(std::list<NetChatroomInfo> *pChatrooms) : mChatrooms(pChatrooms) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LobbyChatroomsMsg", 0);
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
     * Produce a heap copy of this message, which points at the same rooms.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new LobbyChatroomsMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyChatroomsMsgType.
     */
    int Type() override {
        return g_nLobbyChatroomsMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyChatroomsMsg`.
     */
    const char *GetName() const override {
        return "LobbyChatroomsMsg";
    }

    /**
     * Write the rooms.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268f48
     * @ghidraAddress PAL: 0x00272b50
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return The chat rooms. */
    std::list<NetChatroomInfo> *GetChatrooms() const {
        return mChatrooms;
    }

private:
    std::list<NetChatroomInfo> *mChatrooms; /*!< The chat rooms. */
};
