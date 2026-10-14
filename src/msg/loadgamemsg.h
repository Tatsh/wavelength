#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Message that tells the game to load the song of an online session.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d4550`. The class
 * adds no member. The members Clone(), Type(), and GetName() are inline.
 */
class LoadGameMsg : public Message {
public:
    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LoadGameMsg", 0);
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
        return new LoadGameMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLoadGameMsgType.
     */
    int Type() override {
        return g_nLoadGameMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LoadGameMsg`.
     */
    const char *GetName() const override {
        return "LoadGameMsg";
    }
};
