#pragma once

#include "utl/BinStream.h"

/**
 * Stream over a buffer the caller provides.
 *
 * The RTTI records the class as deriving from BinStream. The object is 0x18 bytes. A transfer that
 * would pass the end is cut to the bytes that remain and marks the stream failed.
 */
class BufStream : public BinStream {
public:
    /**
     * Start a stream at the beginning of a buffer. A null buffer makes a failed stream.
     *
     * @param buffer The buffer, or null.
     * @param size The size of the buffer in bytes.
     * @param littleEndian Whether values are stored least significant byte first.
     * @ghidraAddress 0x1002df70
     */
    BufStream(char *buffer, int size, bool littleEndian);

    /**
     * Destroy the stream. The buffer is not released.
     *
     * @ghidraAddress 0x1002e010
     */
    virtual ~BufStream();

    /**
     * Copy bytes out of the buffer.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @ghidraAddress 0x1002e020
     */
    virtual void Read(void *data, int bytes);

    /**
     * Copy bytes into the buffer.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     * @ghidraAddress 0x1002e070
     */
    virtual void Write(const void *data, int bytes);

    /** Do nothing; the buffer holds every byte written. */
    virtual void Flush() {
    }

    /**
     * Move the position, failing the stream when it would leave the buffer.
     *
     * @param offset The offset in bytes.
     * @param type The origin of the offset. Another value leaves the position unchanged.
     * @ghidraAddress 0x1002e0c0
     */
    virtual void Seek(int offset, SeekType type);

    /**
     * @return The position in bytes.
     * @ghidraAddress 0x1002dfb0
     */
    virtual int Tell();

    /**
     * @return Whether the position is at the end of the buffer.
     * @ghidraAddress 0x1002dfc0
     */
    virtual bool Eof();

    /**
     * @return Whether a transfer ran past the end, or the buffer is null.
     * @ghidraAddress 0x1002dfe0
     */
    virtual bool Fail();

private:
    char *mBuffer; /*!< The buffer. */
    bool mFail;    /*!< Whether a transfer ran past the end, or the buffer is null. */
    int mTell;     /*!< The position in bytes. */
    int mSize;     /*!< The size of the buffer in bytes. */
};
