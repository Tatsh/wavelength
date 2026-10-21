#pragma once

#include "game/remixinfo.h"
#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports that the host started to share the remix of the game.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d5ee0`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class ShareRemixBeginMsg : public Message {
public:
    /**
     * Construct a message with the description of the remix.
     *
     * Inline. The constructor does not set mReserved04.
     *
     * @param info The description of the remix.
     */
    explicit ShareRemixBeginMsg(const RemixInfo &info) : mInfo(info) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ShareRemixBeginMsg", 0);
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
        return new ShareRemixBeginMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nShareRemixBeginMsgType.
     */
    int Type() override {
        return g_nShareRemixBeginMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `ShareRemixBeginMsg`.
     */
    const char *GetName() const override {
        return "ShareRemixBeginMsg";
    }

    /**
     * Write the description of the remix.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269070
     * @ghidraAddress PAL: 0x00272c78
     */
    void PrintExtra(PrnStream &stream) const override;

private:
    int mReserved04; // +0x04, the alignment of mInfo to eight bytes.
    RemixInfo mInfo; /*!< The description of the remix. */
};
