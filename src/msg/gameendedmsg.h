#pragma once

#include <vector>

#include "game/netgamescore.h"
#include "msg/message.h"
#include "os/mem.h"

/**
 * Identity that GameEndedMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b80
 */
extern int g_nGameEndedMsgType;

/**
 * Message that reports the end of an online session, with the final scores.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d5e08`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class GameEndedMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039ff40
     * @ghidraAddress PAL: 0x0040ec20
     */
    ~GameEndedMsg() override = default;

    /** Values of mResult. */
    enum Result {
        kResultFinished = 1,       /*!< The song ended normally. */
        kResultHostAborted = 2,    /*!< The host left the session. */
        kResultConnectionLost = 3, /*!< The connection to the session was lost. */
        kResultBooted = 4,         /*!< The host removed this console from the session. */
    };

    /**
     * Construct a message with no scores.
     *
     * Inline.
     *
     * @param nResult One of Result.
     */
    explicit GameEndedMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Construct a message with the final scores.
     *
     * Inline.
     *
     * @param nResult One of Result.
     * @param scores The final score of each player.
     */
    GameEndedMsg(int nResult, const std::vector<NetGameScore> &scores)
        : mResult(nResult), mScores(scores) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "GameEndedMsg", 0);
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
        return new GameEndedMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nGameEndedMsgType.
     */
    int Type() override {
        return g_nGameEndedMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `GameEndedMsg`.
     */
    const char *GetName() const override {
        return "GameEndedMsg";
    }

    /**
     * Write the result, and the scores when there are any.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00269120
     * @ghidraAddress PAL: 0x00272d28
     */
    void PrintExtra(PrnStream &stream) const override;

    /**
     * Copy another message.
     *
     * @param other The message.
     * @ghidraAddress NTSC-U/C: 0x003455e0
     * @ghidraAddress PAL: 0x003b2b18
     */
    GameEndedMsg(const GameEndedMsg &other) = default;

    int mResult;                       /*!< One of Result. */
    std::vector<NetGameScore> mScores; /*!< The final score of each player. */
};
