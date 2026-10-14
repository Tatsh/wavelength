#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Result of NetLobby::DownloadRemix().
 *
 * The RTTI includes the class name and records Message as the base. The vtable is at `0x003d4da0`.
 * The members Clone(), Type(), and GetName() are inline. After a success the downloaded remix is
 * the current remix of TheGameDb.
 */
class RemixDownloadMsg : public Message {
public:
    /**
     * Construct a message.
     *
     * Inline.
     *
     * @param nResult Zero on success, otherwise a negative error.
     * @param pszName The file of the remix, or null after a failure.
     * @param pszNote The note of the remix, or null after a failure.
     */
    RemixDownloadMsg(int nResult, const char *pszName, const char *pszNote)
        : mResult(nResult), mName(pszName), mNote(pszNote) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "RemixDownloadMsg", 0);
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
     * Produce a heap copy of this message, which points at the same texts.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new RemixDownloadMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nRemixDownloadMsgType.
     */
    int Type() override {
        return g_nRemixDownloadMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `RemixDownloadMsg`.
     */
    const char *GetName() const override {
        return "RemixDownloadMsg";
    }

    int mResult;       /*!< Zero on success, otherwise a negative error. */
    const char *mName; /*!< The file of the remix. */
    const char *mNote; /*!< The note of the remix. */
};
