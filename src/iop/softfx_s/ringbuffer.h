#pragma once

/**
 * A byte ring buffer that the writer fills and the reader drains one contiguous block at a time.
 *
 * The module was built without RTTI. The name is inferred from the class's routines.
 *
 * Bytes are written at mWrite and read at mRead, and both wrap from mEnd to mBuffer. Read()
 * returns a block and records its end in mPendingRead, and Release() moves mRead there. The space
 * report holds mReserve bytes back from the writer.
 */
class RingBuffer {
public:
    /**
     * Allocate and clear a buffer of size bytes, rounded down to whole words, with both pointers
     * at its start.
     *
     * @param size Buffer size in bytes.
     * @ghidraAddress NTSC-U/C: 0x00002d70
     * @ghidraAddress PAL: 0x00002d70
     */
    explicit RingBuffer(unsigned int size);

    /**
     * Free the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x00002e00
     * @ghidraAddress PAL: 0x00002e00
     */
    virtual ~RingBuffer();

    /**
     * Record the end-of-stream flag and append bytes, wrapping at mEnd. The routine does not check
     * the free space.
     *
     * @param data Bytes to append, or null to record only the flag.
     * @param size Byte count, or zero to record only the flag.
     * @param endOfStream Whether the writer has no more data.
     * @ghidraAddress NTSC-U/C: 0x000030b0
     * @ghidraAddress PAL: 0x000030b0
     */
    virtual void Write(const void *data, unsigned int size, bool endOfStream);

    /** Report the free space to the writer when it has changed. */
    virtual void Notify() = 0;

    /**
     * Discard every byte by moving both pointers to the start of the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x00003248
     * @ghidraAddress PAL: 0x00003248
     */
    virtual void Reset();

    /**
     * Report whether the stream has ended and every byte has been read.
     *
     * @return True when the write pointer is at mEnd, or when the buffer is empty and the stream
     * has ended.
     * @ghidraAddress NTSC-U/C: 0x00002e68
     * @ghidraAddress PAL: 0x00002e68
     */
    bool IsFinished() const;

    /**
     * Report the space the writer can fill without wrapping, less the reserve.
     *
     * @return Free bytes.
     * @ghidraAddress NTSC-U/C: 0x00002ebc
     * @ghidraAddress PAL: 0x00002ebc
     */
    unsigned int ContiguousFree() const;

    /**
     * Report whether ContiguousFree() is zero. The routine has no caller.
     *
     * @return True when the writer cannot add a byte without wrapping.
     * @ghidraAddress NTSC-U/C: 0x00002f28
     * @ghidraAddress PAL: 0x00002f28
     */
    bool IsFull() const;

    /**
     * Report whether every written byte has been read.
     *
     * @return True when the read and write pointers are equal.
     * @ghidraAddress NTSC-U/C: 0x00002f4c
     * @ghidraAddress PAL: 0x00002f4c
     */
    bool IsEmpty() const;

    /**
     * Notify the writer, then return the next contiguous block of at most maxSize bytes. The read
     * pointer moves when Release() runs.
     *
     * @param maxSize Largest block wanted.
     * @param size Receives the block size.
     * @return The block, or null when the buffer is empty or finished.
     * @ghidraAddress NTSC-U/C: 0x00002f64
     * @ghidraAddress PAL: 0x00002f64
     */
    void *Read(unsigned int maxSize, unsigned int *size);

    /**
     * Consume the block Read() returned and notify the writer.
     *
     * @param block The variable that holds the block. It is cleared. Nothing happens when it is
     * already null.
     * @ghidraAddress NTSC-U/C: 0x00003038
     * @ghidraAddress PAL: 0x00003038
     */
    void Release(void **block);

    /**
     * Report the free space, less the reserve.
     *
     * @return Free bytes.
     * @ghidraAddress NTSC-U/C: 0x000031d0
     * @ghidraAddress PAL: 0x000031d0
     */
    unsigned int FreeSpace() const;

    /**
     * Report the bytes written and not yet read.
     *
     * @param endOfStream Receives the end-of-stream flag.
     * @return Used bytes.
     * @ghidraAddress NTSC-U/C: 0x00003210
     * @ghidraAddress PAL: 0x00003210
     */
    unsigned int Used(bool *endOfStream) const;

    /**
     * Clear the end-of-stream flag. The routine has no caller.
     *
     * @ghidraAddress NTSC-U/C: 0x0000325c
     * @ghidraAddress PAL: 0x0000325c
     */
    void ClearEndOfStream();

    /**
     * Report the end-of-stream flag. The routine has no caller.
     *
     * @return The flag.
     * @ghidraAddress NTSC-U/C: 0x00003264
     * @ghidraAddress PAL: 0x00003264
     */
    bool EndOfStream() const;

protected:
    unsigned int mSize;    /*!< Buffer size in bytes. */
    char *mBuffer;         /*!< Start of the buffer. */
    char *mRead;           /*!< Next byte to read. */
    char *mWrite;          /*!< Next byte to write. */
    char *mEnd;            /*!< End of the buffer. */
    char *mPendingRead;    /*!< End of the block Read() returned, or null. */
    bool mEndOfStream;     /*!< The writer has no more data. */
    unsigned int mReserve; /*!< Bytes the space reports hold back. */
};
