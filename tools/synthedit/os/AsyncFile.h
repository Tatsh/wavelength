#pragma once

#include "os/File.h"
#include "utl/Str.h"

/**
 * Bytes of the buffer of each AsyncFile, from `buf_size` of the `file` configuration array.
 *
 * @ghidraAddress 0x10038bec
 */
extern int gAsyncBufSize;

/**
 * Read the `file` configuration array.
 *
 * @ghidraAddress 0x1000dde0
 */
void AsyncFileInit();

/**
 * Buffered file of the host file system.
 *
 * The RTTI records the class as deriving from File. The object is 0x44 bytes. Reads and writes
 * go through a buffer of #gAsyncBufSize bytes. The reads complete at once on this platform.
 */
class AsyncFile : public File {
public:
    /**
     * Allocate the buffer and open a file.
     *
     * @param name The path.
     * @param mode The open mode, a combination of FileOpenMode bits.
     * @ghidraAddress 0x1000de10
     */
    AsyncFile(const char *name, int mode);

    /**
     * Close the file and free the buffer.
     *
     * @ghidraAddress 0x1000e040
     */
    virtual ~AsyncFile();

    /**
     * Read bytes and wait for them.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @return The number of bytes read, or zero after a failure.
     * @ghidraAddress 0x1000e0e0
     */
    virtual int Read(void *data, int bytes);

    /**
     * Start reading bytes, at most up to the end of the file.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @return Whether the read started.
     * @ghidraAddress 0x1000e130
     */
    virtual bool ReadAsync(void *data, int bytes);

    /**
     * Write bytes through the buffer.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     * @return The number of bytes, or zero after a failure.
     * @ghidraAddress 0x1000e2b0
     */
    virtual int Write(const void *data, int bytes);

    /**
     * Move the position, clamped to the file.
     *
     * @param offset The offset.
     * @param origin The origin, one of BinStream::SeekType.
     * @return The new position.
     * @ghidraAddress 0x1000e3a0
     */
    virtual int Seek(int offset, int origin);

    /**
     * Report the position.
     *
     * @return The position.
     * @ghidraAddress 0x1000e990
     */
    virtual int Tell();

    /**
     * Write the buffer when writing, or refill it when reading.
     *
     * @ghidraAddress 0x1000e430
     */
    virtual void Flush();

    /**
     * Report whether the position is at the end.
     *
     * @return Whether the end was arrived at.
     * @ghidraAddress 0x1000e4e0
     */
    virtual bool Eof();

    /**
     * Report whether an operation failed.
     *
     * @return Whether an operation failed.
     * @ghidraAddress 0x1000e500
     */
    virtual bool Fail();

    /**
     * Report the size.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1000e510
     */
    virtual int Size();

    /**
     * Report the size a `.gz` file records in its last four bytes, or zero for other files.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1000e980
     */
    virtual int UncompressedSize();

    /**
     * Copy buffered bytes of the read ReadAsync() started, refilling the buffer when it runs out.
     *
     * @param bytes Receives the number of bytes read so far.
     * @return Whether the read is complete.
     * @ghidraAddress 0x1000e1b0
     */
    virtual bool ReadDone(int *bytes);

    /**
     * Open a file.
     *
     * @param name The path.
     * @param mode The open mode.
     * @ghidraAddress 0x1000df10
     */
    void Open(const char *name, int mode);

    /**
     * Write out a written file and close it.
     *
     * @ghidraAddress 0x1000e0a0
     */
    void Close();

private:
    /**
     * Open the file of #mFilename.
     *
     * @return The size of the file.
     * @ghidraAddress 0x1000ff00
     */
    int _Open();

    /**
     * Write bytes to the file.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     * @return The number of bytes.
     * @ghidraAddress 0x1000ff50
     */
    int _Write(const void *data, int bytes);

    /**
     * Move the file's position to #mTell.
     *
     * @ghidraAddress 0x1000ff80
     */
    void _SeekToTell();

    /**
     * Read bytes from the file. The read completes before the routine returns.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @ghidraAddress 0x1000ffa0
     */
    void _ReadAsync(void *data, int bytes);

    /**
     * Report whether the last read finished, which it always has.
     *
     * @return Always true.
     * @ghidraAddress 0x1000ffe0
     */
    bool _ReadDone();

    /**
     * Close the file.
     *
     * @ghidraAddress 0x1000fff0
     */
    void _Close();

    int mMode;         /*!< The open mode. */
    int mHandle;       /*!< The `_open` handle, or -1. */
    int mSize;         /*!< The size of the file. */
    int mUCSize;       /*!< The size before compression, or zero. */
    int mTell;         /*!< The position. */
    int mOffset;       /*!< The position in the buffer, #gAsyncBufSize when it is empty. */
    bool mReadStarted; /*!< Whether the buffer was filled for reading. */
    bool mFail;        /*!< Whether an operation failed. */
    char *mBuffer;     /*!< The buffer. */
    char *mData;       /*!< Receives the bytes of the read in progress. */
    int mBytesLeft;    /*!< Bytes of the read in progress still to copy. */
    int mBytesRead;    /*!< Bytes of the read in progress copied so far. */
    String mFilename;  /*!< The path. */
};
