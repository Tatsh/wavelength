#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"
#include "os/string.h"

/**
 * Message that reports the progress of NetInet::Connect().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6480`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class InetConnectStatusMsg : public Message {
public:
    /**
     * Construct a message.
     *
     * Inline.
     *
     * @param status The text of the step under way.
     */
    explicit InetConnectStatusMsg(const String &status) : mStatus(status) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InetConnectStatusMsg", 0);
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
        return new InetConnectStatusMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetConnectStatusMsgType.
     */
    int Type() override {
        return g_nInetConnectStatusMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetConnectStatusMsg`.
     */
    const char *GetName() const override {
        return "InetConnectStatusMsg";
    }

    /**
     * Write the text of the step.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002689e0
     * @ghidraAddress PAL: 0x00272638
     */
    void PrintExtra(PrnStream &stream) const override;

    String mStatus; /*!< The text of the step under way. */
};
