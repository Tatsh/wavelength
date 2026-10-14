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
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039b0d0
     * @ghidraAddress PAL: 0x00409d18
     */
    ~LobbyDisconnectResultMsg() override = default;

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
     * @ghidraAddress NTSC-U/C: 0x0039b178
     * @ghidraAddress PAL: 0x00409dc0
     */
    Message *Clone() override {
        return new LobbyDisconnectResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyDisconnectResultMsgType.
     * @ghidraAddress NTSC-U/C: 0x0039b1b0
     * @ghidraAddress PAL: 0x00409df8
     */
    int Type() override {
        return g_nLobbyDisconnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyDisconnectResultMsg`.
     * @ghidraAddress NTSC-U/C: 0x0039b1c0
     * @ghidraAddress PAL: 0x00409e08
     */
    const char *GetName() const override {
        return "LobbyDisconnectResultMsg";
    }
};
