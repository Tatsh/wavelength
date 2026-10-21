#pragma once

#ifdef VIDEO_STANDARD_PAL

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports whether the DNAS authentication of the lobby connection is running.
 *
 * The RTTI includes the class name and records Message as the base. The object is 8 bytes and its
 * vtable is at `0x0046aca0`. DNASTask sends it to the receiver of the lobby connection, and
 * NetLobbyConnect shows the DNAS notice while it runs.
 */
class DNASStatusMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x00405948
     */
    ~DNASStatusMsg() override = default;

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "DNASStatusMsg", 0);
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
     * Construct a message.
     *
     * Inline.
     *
     * @param nRunning Non-zero while the authentication runs.
     */
    explicit DNASStatusMsg(int nRunning) : mRunning(nRunning) {
    }

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     * @ghidraAddress PAL: 0x004059f0
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nDNASStatusMsgType.
     * @ghidraAddress PAL: 0x00405a40
     */
    int Type() override;

    /**
     * Report this message's class name.
     *
     * @return The literal `DNASStatusMsg`.
     * @ghidraAddress PAL: 0x00405a50
     */
    const char *GetName() const override;

    /** @return Non-zero while the authentication runs. */
    int GetRunning() const {
        return mRunning;
    }

private:
    int mRunning; /*!< Non-zero while the authentication runs. */
};

#endif
