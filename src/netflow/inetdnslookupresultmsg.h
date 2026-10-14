#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that reports the outcome of NetInet::LookupHost().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d63f0`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class InetDNSLookupResultMsg : public Message {
public:
    /** The value of mResult after a failed lookup. */
    static constexpr int kResultFailed = -90;

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult The outcome, zero on success.
     * @param pszAddress The address of the host as text, or empty.
     */
    InetDNSLookupResultMsg(int nResult, const char *pszAddress)
        : mResult(nResult), mAddress(pszAddress) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InetDNSLookupResultMsg", 0);
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
     * Produce a heap copy of this message, which points at the same address text.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new InetDNSLookupResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetDNSLookupResultMsgType.
     */
    int Type() override {
        return g_nInetDNSLookupResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetDNSLookupResultMsg`.
     */
    const char *GetName() const override {
        return "InetDNSLookupResultMsg";
    }

    /**
     * Write the result and the address.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268aa0
     * @ghidraAddress PAL: 0x002726a8
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;          /*!< The outcome, zero on success. */
    const char *mAddress; /*!< The address of the host, as text. */
};
