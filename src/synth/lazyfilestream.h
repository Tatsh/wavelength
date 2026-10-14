#pragma once

#include "os/file.h"
#include "os/string.h"

/**
 * Double-buffered reader with one asynchronous read of a file always in flight.
 *
 * The class has no RTTI, and its name comes from the allocation tag of its buffers. The object is
 * 0x20 bytes. While the caller consumes one buffer, the next read fills the other.
 */
class LazyFileStream {
public:
    /**
     * Open a file, allocate the buffers, and start the first read.
     *
     * @param filename The path of the file.
     * @param nBufferSize The size of each buffer in bytes.
     * @ghidraAddress NTSC-U/C: 0x002517f0
     * @ghidraAddress PAL: 0x0025a198
     */
    LazyFileStream(const String &filename, int nBufferSize);

    /**
     * Close the file and free the buffers.
     *
     * @ghidraAddress NTSC-U/C: 0x002518a8
     * @ghidraAddress PAL: 0x0025a250
     */
    ~LazyFileStream();

    /**
     * Report whether the last read reached the end of the file.
     *
     * @return Whether no read remains.
     * @ghidraAddress NTSC-U/C: 0x00251940
     * @ghidraAddress PAL: 0x0025a2e8
     */
    bool Eof() const;

    /**
     * Report whether the read in flight is complete, recording how many bytes arrived.
     *
     * @return Whether the read is complete.
     * @ghidraAddress NTSC-U/C: 0x00251990
     * @ghidraAddress PAL: 0x0025a338
     */
    bool ReadDone();

    /**
     * Take the buffer of the completed read, and start the next read unless the file ended.
     *
     * @param pnPosition Receives the offset of the buffer's data in the file.
     * @param pnBytesAvailable Receives the number of bytes in the buffer.
     * @return The buffer.
     * @ghidraAddress NTSC-U/C: 0x002519e0
     * @ghidraAddress PAL: 0x0025a388
     */
    char *GetData(int *pnPosition, int *pnBytesAvailable);

private:
    /** The number of buffers. */
    static constexpr unsigned int kNumBuffers = 2;

    /**
     * Start reading into the current buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x00251950
     * @ghidraAddress PAL: 0x0025a2f8
     */
    void StartRead();

    int mMoreData;               /*!< Whether a read remains. */
    int mPosition;               /*!< Offset in the file of the next data handed out. */
    File *mFile;                 /*!< The file. */
    char *mBuffers[kNumBuffers]; /*!< The buffers. */
    unsigned int mCurBuffer;     /*!< Index of the buffer of the read in flight. */
    int mBufferSize;             /*!< Size of each buffer in bytes. */
    int mBytesAvailable;         /*!< Bytes of the completed read, or -1 while it is in flight. */
};
