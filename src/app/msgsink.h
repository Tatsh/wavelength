#pragma once

#include <cstddef>

#include "os/mem.h"

class Message;

/**
 * Receiver of engine messages.
 *
 * The RTTI includes the class name and records no base. The class declares no data member, and
 * the vptr sits at offset 0 over a four-byte subobject. The vtable at `0x003c7708` runs the type
 * function, the destructor, Dispatch(), and DispatchPriv(), whose entry is the shared pure-virtual
 * stub.
 *
 * Both members report whether the sink handled the message. MsgSource::SendUntilHandled() stops
 * at the first sink that reports a message as handled, and the front end passes a message on only
 * when a sink does not handle it.
 */
class MsgSink {
public:
    /**
     * Allocate a sink from the pool heaps under the tag "MsgSink".
     *
     * No out-of-line body exists. Every allocation of a derived class inlines the call with the
     * default alignment, among them the one in InputMgr's constructor at `0x001182a8`.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "MsgSink", 0);
    }

    /**
     * Return a sink to the pool heaps.
     *
     * No out-of-line body exists. The release branch of every derived destructor inlines the call,
     * among them the one in InputMgr's destructor at `0x00118700`.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00338948
     * @ghidraAddress PAL: 0x003a5ef8
     */
    virtual ~MsgSink();

    /**
     * Accept a message.
     *
     * The body calls DispatchPriv() through the object's own vptr and reports its result. Classes
     * that route a message elsewhere first override it.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00333eb0
     * @ghidraAddress PAL: 0x003a1460
     */
    virtual bool Dispatch(Message *pMsg);

    /**
     * Act on a message.
     *
     * Public rather than protected, because some sinks pass a message to the DispatchPriv() of
     * another sink they store.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     */
    virtual bool DispatchPriv(Message *pMsg) = 0;
};
