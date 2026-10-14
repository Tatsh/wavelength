#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports the outcome of NetInet::CheckConnection().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d4360`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class InetCheckConnectionResultMsg : public Message {
public:
    /** The value of mResult when the connection is down. */
    static constexpr int kResultDown = -91;

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult The outcome, zero while the connection works.
     */
    explicit InetCheckConnectionResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InetCheckConnectionResultMsg", 0);
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
        return new InetCheckConnectionResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetCheckConnectionResultMsgType.
     */
    int Type() override {
        return g_nInetCheckConnectionResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetCheckConnectionResultMsg`.
     */
    const char *GetName() const override {
        return "InetCheckConnectionResultMsg";
    }

    int mResult; /*!< The outcome, zero while the connection works. */
};
