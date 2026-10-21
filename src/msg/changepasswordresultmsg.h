#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Result of NetLobby::ChangePassword().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class ChangePasswordResultMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039e690
     * @ghidraAddress PAL: 0x0040d370
     */
    ~ChangePasswordResultMsg() override = default;

    /** Values of mResult. */
    enum Result {
        kResultUpdateFailed = -77, /*!< The server did not update the password. */
        kResultSuccess = 0,        /*!< The password was changed. */
    };

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult One of Result, or another negative error.
     */
    explicit ChangePasswordResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ChangePasswordResultMsg", 0);
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
        return new ChangePasswordResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nChangePasswordResultMsgType.
     */
    int Type() override {
        return g_nChangePasswordResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `ChangePasswordResultMsg`.
     */
    const char *GetName() const override {
        return "ChangePasswordResultMsg";
    }

    /**
     * Write the result.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268be8
     * @ghidraAddress PAL: 0x002727f0
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return One of Result, or another negative error. */
    int GetResult() const {
        return mResult;
    }

private:
    int mResult; /*!< One of Result, or another negative error. */
};
