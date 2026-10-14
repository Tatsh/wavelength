#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Report that NetLobby::Disconnect() finished.
 *
 * The RTTI includes the class name and records Message as the base. The vtable is at `0x003d4de8`.
 * The members Clone(), Type(), and GetName() are inline. The readers here use only its identity.
 */
class LobbyDisconnectResultMsg : public Message {
public:
    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LobbyDisconnectResultMsg", 0);
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
        return new LobbyDisconnectResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyDisconnectResultMsgType.
     */
    int Type() override {
        return g_nLobbyDisconnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyDisconnectResultMsg`.
     */
    const char *GetName() const override {
        return "LobbyDisconnectResultMsg";
    }
};
