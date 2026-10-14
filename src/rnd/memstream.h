#pragma once

#include <vector>

#include "rnd/stream.h"

namespace Rnd {

/** Bytes the constructor reserves before the first write. */
constexpr int kMemStreamReserve = 0x1000;

/**
 * Stream over a buffer the stream owns and grows.
 *
 * Its RTTI descriptor is at `0x008ef190`. It has single inheritance from `Rnd::Stream` at offset 0.
 * The object is 0x1c bytes and its vtable is at `0x00826178`. The end and failure states are stored
 * rather than derived. Storing them separates this class from `Rnd::BufStream`.
 */
class MemStream : public Stream {
public:
    /**
     * Construct an empty stream with kMemStreamReserve bytes reserved.
     */
    MemStream();

    virtual ~MemStream();

    virtual Stream &Read(void *pDest, int nSize);

    virtual Stream &Write(const void *pSrc, int nSize);

    virtual Stream &Flush();

    virtual Stream &Seek(int nOffset, int nWhence);

    virtual int Tell();

    virtual int Eof();

    virtual int Fail();

    /**
     * Drop the bytes already read from the front of the buffer and rewind the cursor.
     *
     * The unread tail moves to the start of the buffer, and the buffer shrinks by the old cursor
     * position. No call site survives in the shipped program.
     */
    void Compact();

private:
    int mEof;                  // +0x04
    int mFail;                 // +0x08
    int mPos;                  // +0x0c
    std::vector<char> mBuffer; // +0x10
};

} // namespace Rnd
