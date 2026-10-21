#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports that the online launchpad of this console ended before the game started.
 *
 * The RTTI includes the class name and records Message as the base. The vtable is at
 * `0x003d5f70`. The payload is the reason, a negative error code of the session or 0 for none.
 * The members Clone(), Type(), and GetName() are inline.
 */
class LaunchpadAbortedMsg : public Message {
public:
    /** Values of mReason. The names of the codes without an error token are inferred. */
    enum Reason {
        kReasonInit = -100,            /*!< The session library could not start. */
        kReasonServer = -89,           /*!< A request to the game server failed. */
        kReasonLost = -82,             /*!< The connection to the game server was lost. */
        kReasonHostFailed = -73,       /*!< The session could not be hosted. */
        kReasonConnectFailed = -72,    /*!< The connection to the host failed. */
        kReasonCreateFailed = -71,     /*!< The game server refused the game of the host. */
        kReasonIncompatibleHost = -70, /*!< The `net_lpad_incompatible_host` error. */
        kReasonFull = -69,             /*!< The `net_lpad_full` error. */
        kReasonInProgress = -68,       /*!< The `net_lpad_in_progress` error. */
        kReasonDenied = -67,           /*!< The `net_lpad_denied` error. */
        kReasonIncompatible = -66,     /*!< The `net_lpad_incompatible` error. */
        kReasonDied = -65,             /*!< The `net_lpad_abort_died` error. */
        kReasonHost = -64,             /*!< The `net_lpad_abort_host` error. */
        kReasonBoot = -63,             /*!< The `net_lpad_abort_boot` error. */
        kReasonBad = -62,              /*!< The `net_lpad_abort_bad` error. */
        kReasonNone = 0,               /*!< The launchpad did not end. */
    };

    /**
     * Construct a message with a reason.
     *
     * Inline.
     *
     * @param nReason One of Reason, or another error code.
     */
    explicit LaunchpadAbortedMsg(int nReason) : mReason(nReason) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LaunchpadAbortedMsg", 0);
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
        return new LaunchpadAbortedMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLaunchpadAbortedMsgType.
     */
    int Type() override {
        return g_nLaunchpadAbortedMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LaunchpadAbortedMsg`.
     */
    const char *GetName() const override {
        return "LaunchpadAbortedMsg";
    }

    /**
     * Write the reason.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269008
     * @ghidraAddress PAL: 0x00272c10
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return One of Reason, or another error code. */
    int GetReason() const {
        return mReason;
    }

private:
    int mReason; /*!< One of Reason, or another error code. */
};
