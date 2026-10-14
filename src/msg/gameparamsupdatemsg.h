#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netgameparams.h"
#include "os/mem.h"

/**
 * Message that reports the settings of the online game the host published.
 *
 * The RTTI includes the class name and records Message as the base. The vtable is at
 * `0x003d5f28`. The members Clone(), Type(), and GetName() are inline.
 */
class GameParamsUpdateMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039f9f8
     * @ghidraAddress PAL: 0x0040e6d8
     */
    ~GameParamsUpdateMsg() override = default;

    /**
     * Construct a message with the settings.
     *
     * Inline.
     *
     * @param params The settings of the game.
     */
    explicit GameParamsUpdateMsg(const NetGameParams &params) : mParams(params) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "GameParamsUpdateMsg", 0);
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
        return new GameParamsUpdateMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nGameParamsUpdateMsgType.
     */
    int Type() override {
        return g_nGameParamsUpdateMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `GameParamsUpdateMsg`.
     */
    const char *GetName() const override {
        return "GameParamsUpdateMsg";
    }

    /**
     * Write the settings.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269048
     */
    void PrintExtra(PrnStream &stream) const override;

    NetGameParams mParams; /*!< The settings of the game. */
};
