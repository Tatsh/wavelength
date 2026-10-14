#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports how much of the remix of an online game the consoles have exchanged.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d5e98`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class ShareRemixProgressMsg : public Message {
public:
    /**
     * Construct a message with the progress.
     *
     * Inline.
     *
     * @param fProgress The part exchanged, from 0 to 1.
     * @param nReceived The share-only flag of the remix, from ShareRemixPkt::mShareOnly.
     */
    ShareRemixProgressMsg(float fProgress, int nReceived)
        : mProgress(fProgress), mReceived(nReceived) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ShareRemixProgressMsg", 0);
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
        return new ShareRemixProgressMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nShareRemixProgressMsgType.
     */
    int Type() override {
        return g_nShareRemixProgressMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `ShareRemixProgressMsg`.
     */
    const char *GetName() const override {
        return "ShareRemixProgressMsg";
    }

    /**
     * Write the progress and mReceived.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269098
     * @ghidraAddress PAL: 0x00272ca0
     */
    void PrintExtra(PrnStream &stream) const override;

    float mProgress; /*!< The part exchanged, from 0 to 1. */
    int mReceived;   /*!< The share-only flag of the remix, non-zero for a remix only shared. */
};
