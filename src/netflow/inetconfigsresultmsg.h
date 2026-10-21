#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/inetconfig.h"
#include "os/mem.h"

/**
 * Identity that InetConfigsResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b04
 * @ghidraAddress PAL: 0x0042f9d0
 */
extern int g_nInetConfigsResultMsgType;

/**
 * Message that reports the network configurations NetInet::RequestConfigs() found.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d64c8`. The
 * destructor, Clone(), Type(), and GetName() are inline.
 */
class InetConfigsResultMsg : public Message {
public:
    /**
     * Construct a result with no configurations.
     *
     * Inline.
     *
     * @param nResult The outcome, negative when the search failed.
     */
    explicit InetConfigsResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "InetConfigsResultMsg", 0);
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
     * Release the message and its configurations.
     *
     * @ghidraAddress NTSC-U/C: 0x0039dc98
     * @ghidraAddress PAL: 0x0040c9c0
     */
    ~InetConfigsResultMsg() override = default;

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0039dd70
     * @ghidraAddress PAL: 0x0040ca98
     */
    Message *Clone() override {
        return new InetConfigsResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetConfigsResultMsgType.
     * @ghidraAddress NTSC-U/C: 0x0039ddb0
     * @ghidraAddress PAL: 0x0040cad8
     */
    int Type() override {
        return g_nInetConfigsResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `InetConfigsResultMsg`.
     * @ghidraAddress NTSC-U/C: 0x0039ddc0
     * @ghidraAddress PAL: 0x0040cae8
     */
    const char *GetName() const override {
        return "InetConfigsResultMsg";
    }

    /**
     * Write the result, and the configurations after a success.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268970
     * @ghidraAddress PAL: 0x002725c8
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return The configurations found. */
    const std::list<InetConfig> &GetConfigs() const {
        return mConfigs;
    }

    /** @return The configurations found, for filling. */
    std::list<InetConfig> &GetConfigs() {
        return mConfigs;
    }

    /** @return The outcome, negative when the search failed. */
    int GetResult() const {
        return mResult;
    }

private:
    int mResult;                    /*!< The outcome, negative when the search failed. */
    std::list<InetConfig> mConfigs; /*!< The configurations found. */
};
