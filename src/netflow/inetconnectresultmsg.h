#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"
#include "os/string.h"

/**
 * Message that reports the outcome of NetInet::Connect().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6438`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class InetConnectResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultSuccess = 0,              /*!< The connection is up. */
        kResultOpenFailed = -100,        /*!< The configuration could not be opened. */
        kResultConfigNotFound = -99,     /*!< The configuration is missing from the card. */
        kResultConfigWrongConsole = -98, /*!< The configuration belongs to another console. */
        kResultConfigError = -97,        /*!< The configuration cannot be read. */
        kResultNoCombination = -96,      /*!< The list has no combination of that number. */
        kResultHardwareError = -95,      /*!< The network adaptor failed. */
        kResultTimeout = -94,            /*!< The connection timed out. */
        kResultFailed = -93,             /*!< The interface reported a failure. */
#ifdef VIDEO_STANDARD_PAL
        kResultAuthFailed = -92, /*!< The provider refused the login. */
#endif
    };

    /**
     * Construct a result with no error text.
     *
     * Inline.
     *
     * @param nResult One of Result, or another failure.
     */
    explicit InetConnectResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InetConnectResultMsg", 0);
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
        return new InetConnectResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetConnectResultMsgType.
     */
    int Type() override {
        return g_nInetConnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetConnectResultMsg`.
     */
    const char *GetName() const override {
        return "InetConnectResultMsg";
    }

    /**
     * Write the result, and the error text when it is not empty.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268a10
     * @ghidraAddress PAL: 0x00272668
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;   /*!< The outcome, one of Result or another failure. */
    String mError; /*!< The text of the error, or empty. */
};
