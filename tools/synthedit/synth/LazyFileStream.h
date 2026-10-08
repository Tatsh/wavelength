#pragma once

#include "os/File.h"
#include "utl/Str.h"

/**
 * Double-buffered reader with one asynchronous read of a file always in flight.
 *
 * The class has no RTTI, and its name comes from its source file. The object is 0x20 bytes.
 * While the caller consumes one buffer, the next read fills the other.
 */
class LazyFileStream {
public:
    /**
     * Open a file, allocate the buffers, and start the first read.
     *
     * @param filename The path of the file.
     * @param bufferSize The size of each buffer in bytes.
     * @ghidraAddress 0x1002ccf0
     */
    LazyFileStream(const String &filename, int bufferSize);

    /**
     * Close the file and free the buffers.
     *
     * @ghidraAddress 0x1002cdde
     */
    ~LazyFileStream();

    /**
     * Report whether the last read reached the end of the file.
     *
     * @return Whether no read remains.
     * @ghidraAddress 0x1002ce6c
     */
    bool Eof() const;

    /**
     * Report whether the read in flight is complete, recording how many bytes arrived.
     *
     * @return Whether the read is complete.
     * @ghidraAddress 0x1002cf0c
     */
    bool ReadDone();

    /**
     * Take the buffer of the completed read, and start the next read unless the file ended.
     *
     * @param position Receives the offset of the buffer's data in the file.
     * @param bytesAvailable Receives the number of bytes in the buffer.
     * @return The buffer.
     * @ghidraAddress 0x1002cf49
     */
    char *GetData(int *position, int *bytesAvailable);

private:
    /** The number of buffers. */
    enum { kNumBuffers = 2 };

    /**
     * Start reading into the current buffer.
     *
     * @ghidraAddress 0x1002ce85
     */
    void StartRead();

    bool mMoreData;              /*!< Whether a read remains. */
    int mPosition;               /*!< Offset in the file of the next data handed out. */
    File *mFile;                 /*!< The file. */
    char *mBuffers[kNumBuffers]; /*!< The buffers. */
    unsigned int mCurBuffer;     /*!< Index of the buffer of the read in flight. */
    int mBufferSize;             /*!< Size of each buffer in bytes. */
    int mBytesAvailable;         /*!< Bytes of the completed read, or -1 while it is in flight. */
};
