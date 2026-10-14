#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netreporemix.h"
#include "os/mem.h"

/**
 * Message that reports the remixes NetLobby::RequestRemixes() found in the online repository.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6048`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class RepoRemixesMsg : public Message {
public:
    /**
     * Construct a message.
     *
     * Inline.
     *
     * @param nResult The outcome, zero on success.
     * @param pRemixes The remixes, or null after a failure. The caller retains them.
     */
    RepoRemixesMsg(int nResult, std::list<NetRepoRemix> *pRemixes)
        : mResult(nResult), mRemixes(pRemixes) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "RepoRemixesMsg", 0);
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
     * Produce a heap copy of this message, which points at the same remixes.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new RepoRemixesMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nRepoRemixesMsgType.
     */
    int Type() override {
        return g_nRepoRemixesMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `RepoRemixesMsg`.
     */
    const char *GetName() const override {
        return "RepoRemixesMsg";
    }

    /**
     * Write the result, and the remixes when the list exists.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268d98
     * @ghidraAddress PAL: 0x002729a0
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;                       /*!< The outcome, zero on success. */
    std::list<NetRepoRemix> *mRemixes; /*!< The remixes found. */
};
