#pragma once

#include <stddef.h>

#include "utl/MemMgr.h"
#include "utl/Message.h"

/**
 * Receiver of the messages of a MsgSource.
 *
 * The RTTI records the class. The object is 4 bytes. A derived class provides DispatchPriv().
 */
class MsgSink {
public:
    /** Allocate from the main heap under the name `MsgSink`. */
    void *operator new(size_t size) {
        return MemAlloc(static_cast<int>(size), "MsgSink", 0);
    }

    /**
     * Return memory to its heap.
     *
     * @ghidraAddress 0x10014070
     */
    void operator delete(void *mem) {
        MemFree(mem);
    }

    /**
     * Destroy the sink.
     *
     * @ghidraAddress 0x10013620
     */
    virtual ~MsgSink();

    /**
     * Handle a message.
     *
     * @param msg The message.
     * @return Whether the sink handled the message.
     * @ghidraAddress 0x10013630
     */
    virtual bool Dispatch(Message *msg);

    /**
     * Handle a message in the derived class.
     *
     * @param msg The message.
     * @return Whether the sink handled the message.
     */
    virtual bool DispatchPriv(Message *msg) = 0;
};
