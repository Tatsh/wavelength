#pragma once

#include "rnd/stream.h"

namespace Rnd {

/**
 * Stream over a fixed buffer the caller owns.
 *
 * Its RTTI descriptor is at `0x008ef1e0`. It has single inheritance from `Rnd::Stream` at offset 0.
 * The object is 0x14 bytes and its vtable is at `0x008260c0`.
 *
 * The buffer neither grows nor is released here. A transfer that would pass mSize is shortened to
 * the remainder and sets mFail, so an overrun reports itself rather than corrupting memory. Slot
 * 10 of the vtable is null, so a BufStream must not be destroyed through a `Rnd::Stream` pointer.
 */
class BufStream : public Stream {
public:
    /**
     * Construct a stream over nSize bytes at pBuffer, positioned at the start.
     *
     * A null buffer starts the stream failed. RndAsyncLoader::PollAsyncLoads() builds one over
     * each completed read.
     *
     * @param pBuffer The buffer, which the caller retains.
     * @param nSize The buffer size in bytes.
     */
    BufStream(char *pBuffer, int nSize);

    virtual Stream &Read(void *pDest, int nSize);

    virtual Stream &Write(const void *pSrc, int nSize);

    virtual Stream &Flush();

    virtual Stream &Seek(int nOffset, int nWhence);

    virtual int Tell();

    virtual int Eof();

    virtual int Fail();

private:
    char *mBuffer; // +0x04
    int mFail;     // +0x08
    int mPos;      // +0x0c
    int mSize;     // +0x10
};

} // namespace Rnd
