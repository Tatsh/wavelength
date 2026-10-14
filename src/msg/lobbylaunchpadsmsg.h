#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpadinfo.h"
#include "os/mem.h"

/**
 * List of launchpads, the result of NetLobby::RequestLaunchpads().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class LobbyLaunchpadsMsg : public Message {
public:
    /**
     * Construct a message that points at the launchpads.
     *
     * Inline.
     *
     * @param pLaunchpads The launchpads. The caller retains them.
     * @param nHasPrevious Non-zero when launchpads precede the list.
     */
    LobbyLaunchpadsMsg(std::list<NetLaunchpadInfo> *pLaunchpads, int nHasPrevious)
        : mLaunchpads(pLaunchpads), mHasPrevious(nHasPrevious) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LobbyLaunchpadsMsg", 0);
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
     * Produce a heap copy of this message, which points at the same launchpads.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new LobbyLaunchpadsMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyLaunchpadsMsgType.
     */
    int Type() override {
        return g_nLobbyLaunchpadsMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyLaunchpadsMsg`.
     */
    const char *GetName() const override {
        return "LobbyLaunchpadsMsg";
    }

    /**
     * Write the launchpads when the list exists.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268f70
     * @ghidraAddress PAL: 0x00272b78
     */
    void PrintExtra(PrnStream &stream) const override;

    std::list<NetLaunchpadInfo> *mLaunchpads; /*!< The launchpads. */
    int mHasPrevious;                         /*!< Non-zero when launchpads precede the list. */
};
