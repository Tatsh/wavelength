#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Result of NetLobby::DeleteAccount().
 *
 * The RTTI includes the class name and records Message as the base. The vtable is at `0x003d62d0`.
 * The members Clone(), Type(), and GetName() are inline.
 */
class DeleteAccountResultMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039e580
     * @ghidraAddress PAL: 0x0040d260
     */
    ~DeleteAccountResultMsg() override = default;

    /** Values of mResult. */
    enum Result {
        kResultDeleteFailed = -79, /*!< The server did not delete the account. */
        kResultSuccess = 0,        /*!< The account was deleted. */
    };

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult Zero on success, otherwise a negative error.
     */
    explicit DeleteAccountResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "DeleteAccountResultMsg", 0);
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
        return new DeleteAccountResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nDeleteAccountResultMsgType.
     */
    int Type() override {
        return g_nDeleteAccountResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `DeleteAccountResultMsg`.
     */
    const char *GetName() const override {
        return "DeleteAccountResultMsg";
    }

    /**
     * Write the result.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268ba8
     * @ghidraAddress PAL: 0x002727b0
     */
    void PrintExtra(PrnStream &stream) const override;

private:
    int mResult; /*!< Zero on success, otherwise a negative error. */
};
