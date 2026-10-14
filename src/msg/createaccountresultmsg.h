#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Result of NetLobby::CreateAccount().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class CreateAccountResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultAlreadyExists = -81,      /*!< An account of the name exists. */
        kResultRegistrationFailed = -80, /*!< The server refused the registration. */
        kResultVulgarName = -76,         /*!< The name is not acceptable. */
        kResultSuccess = 0,              /*!< The account was created. */
    };

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult One of Result, or another negative error.
     */
    explicit CreateAccountResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "CreateAccountResultMsg", 0);
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
        return new CreateAccountResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nCreateAccountResultMsgType.
     */
    int Type() override {
        return g_nCreateAccountResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `CreateAccountResultMsg`.
     */
    const char *GetName() const override {
        return "CreateAccountResultMsg";
    }

    /**
     * Write the result.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268b68
     * @ghidraAddress PAL: 0x00272770
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult; /*!< One of Result, or another negative error. */
};
