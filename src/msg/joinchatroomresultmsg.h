#pragma once

#include "msg/message.h"
#include "netflow/netchatroominfo.h"
#include "os/mem.h"

/**
 * Identity that JoinChatroomResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b4c
 * @ghidraAddress PAL: 0x0042fa1c
 */
extern int g_nJoinChatroomResultMsgType;

/**
 * Message that reports the result of joining a chat room of the online lobby.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d60d8`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class JoinChatroomResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultVulgarName = -76, /*!< The chatroom name is not acceptable. */
        kResultFailed = -75,     /*!< The server refused the request. */
        kResultNameExists = -74, /*!< A chatroom of the name exists. */
        kResultSuccess = 0,      /*!< The player is in the chatroom. */
    };

    /**
     * Construct a failure, whose room has the identifier -1.
     *
     * Inline. The counts of the room are not set.
     *
     * @param nResult The error.
     */
    explicit JoinChatroomResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Construct a result with the room joined.
     *
     * Inline.
     *
     * @param nResult One of Result.
     * @param chatroom The room.
     */
    JoinChatroomResultMsg(int nResult, const NetChatroomInfo &chatroom)
        : mResult(nResult), mChatroom(chatroom) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "JoinChatroomResultMsg", 0);
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
        return new JoinChatroomResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nJoinChatroomResultMsgType.
     */
    int Type() override {
        return g_nJoinChatroomResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `JoinChatroomResultMsg`.
     */
    const char *GetName() const override {
        return "JoinChatroomResultMsg";
    }

    /**
     * Write the result, and the room after a success.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268cd0
     * @ghidraAddress PAL: 0x002728d8
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;               /*!< Zero on success, otherwise an error code. */
    NetChatroomInfo mChatroom; /*!< The room joined. */
};
