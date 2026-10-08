#pragma once

#include "os/File.h"
#include "utl/Str.h"

/**
 * Read-only file inside the archive, read through the block manager.
 *
 * The RTTI records the class as deriving from File. The object is 0x40 bytes.
 */
class ArkFile : public File {
public:
    /**
     * Find a file in the archive. Writing fails.
     *
     * @param name The path.
     * @param mode The open mode, a combination of FileOpenMode bits.
     * @ghidraAddress 0x1000e590
     */
    ArkFile(const char *name, int mode);

    /**
     * Drop the reads still waiting for blocks.
     *
     * @ghidraAddress 0x1000e660
     */
    virtual ~ArkFile();

    /**
     * Read bytes and wait for them.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @return The number of bytes read, or zero when the read did not start.
     * @ghidraAddress 0x1000e6c0
     */
    virtual int Read(void *data, int bytes);

    /**
     * Queue a read of each block the bytes cover, at most up to the end of the file.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @return Whether the read started. A read fails to start at the end of the file or while
     * another read is in progress.
     * @ghidraAddress 0x1000e710
     */
    virtual bool ReadAsync(void *data, int bytes);

    /**
     * Fail, because the archive is read-only.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     * @return Zero.
     * @ghidraAddress 0x1000e8e0
     */
    virtual int Write(const void *data, int bytes);

    /**
     * Move the position. The position is not clamped.
     *
     * @param offset The offset.
     * @param origin The origin, one of BinStream::SeekType.
     * @return The new position.
     * @ghidraAddress 0x1000e900
     */
    virtual int Seek(int offset, int origin);

    /**
     * Report the position.
     *
     * @return The position.
     * @ghidraAddress 0x1000e940
     */
    virtual int Tell();

    /**
     * Do nothing. The linker merged the body with DebugPrint().
     *
     * @ghidraAddress 0x1000fb60
     */
    virtual void Flush();

    /**
     * Report whether the position is at the end.
     *
     * @return Whether the end was arrived at.
     * @ghidraAddress 0x1000e950
     */
    virtual bool Eof();

    /**
     * Report whether the file was not found or was opened for writing.
     *
     * @return Whether the file failed.
     * @ghidraAddress 0x1000e970
     */
    virtual bool Fail();

    /**
     * Report the size in the archive.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1000e980
     */
    virtual int Size();

    /**
     * Report the size before compression.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1000e990
     */
    virtual int UncompressedSize();

    /**
     * Poll the block manager and report whether every block of the read arrived.
     *
     * @param bytes Receives the number of bytes read when the read is complete.
     * @return Whether the read is complete.
     * @ghidraAddress 0x1000e9c0
     */
    virtual bool ReadDone(int *bytes);

    /**
     * Count the bytes one block delivered.
     *
     * @param bytes The number of bytes.
     * @ghidraAddress 0x1000e9a0
     */
    void TaskDone(int bytes);

private:
    int mReserved04;     // +0x04, not written by any routine recovered so far.
    __int64 mArcOffset;  /*!< Offset of the file in the archive. */
    int mSize;           /*!< Size of the file in the archive. */
    int mUCSize;         /*!< Size of the file before compression. */
    int mNumOutstanding; /*!< Blocks of the read in progress still to arrive. */
    int mBytesRead;      /*!< Bytes of the read in progress delivered so far. */
    int mTell;           /*!< The position. */
    int mFail;           /*!< Whether the file failed. */
    String mFilename;    /*!< Never assigned. */
};
