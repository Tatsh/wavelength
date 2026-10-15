#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports that NetInet::Disconnect() finished.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d43a8`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class InetDisconnectResultMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00397588
     * @ghidraAddress PAL: 0x00405e10
     */
    ~InetDisconnectResultMsg() override = default;

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InetDisconnectResultMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x00397630
     * @ghidraAddress PAL: 0x00405eb8
     */
    Message *Clone() override {
        return new InetDisconnectResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetDisconnectResultMsgType.
     * @ghidraAddress NTSC-U/C: 0x00397668
     * @ghidraAddress PAL: 0x00405ef0
     */
    int Type() override {
        return g_nInetDisconnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetDisconnectResultMsg`.
     * @ghidraAddress NTSC-U/C: 0x00397678
     * @ghidraAddress PAL: 0x00405f00
     */
    const char *GetName() const override {
        return "InetDisconnectResultMsg";
    }
};
