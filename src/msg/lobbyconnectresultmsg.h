#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbylocation.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Result of NetLobby::Connect().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class LobbyConnectResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultFailed = -100,       /*!< The network library failed. */
        kResultLostInternet = -91,  /*!< The network connection was lost. */
        kResultLookupFailed = -90,  /*!< The name of the server did not resolve. */
        kResultSessionFailed = -89, /*!< The lobby session did not begin. */
        kResultConnectFailed = -88, /*!< The lobby server did not connect. */
        kResultDNASFailed = -60,    /*!< The DNAS authentication failed, in the European release. */
        kResultSuccess = 0,         /*!< The lobby server is connected. */
    };

    /**
     * Construct a result with no locations.
     *
     * Inline.
     *
     * @param nResult One of Result, or another negative error.
     */
    explicit LobbyConnectResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LobbyConnectResultMsg", 0);
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
        return new LobbyConnectResultMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyConnectResultMsgType.
     */
    int Type() override {
        return g_nLobbyConnectResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyConnectResultMsg`.
     */
    const char *GetName() const override {
        return "LobbyConnectResultMsg";
    }

    /**
     * Write the result, and the locations after a success.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268af8
     * @ghidraAddress PAL: 0x00272700
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;                         /*!< One of Result, or another negative error. */
    std::list<LobbyLocation> mLocations; /*!< The locations of the server. */
};
