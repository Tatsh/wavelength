#pragma once

#include "os/file.h"
#include "os/string.h"

/**
 * File of the host or the disc file system, read and written through a buffer with
 * non-blocking I/O.
 *
 * The RTTI includes the class name and records File as the base. The size before compression of a
 * `.gz` file opened to read is its last four bytes.
 */
class AsyncFile : public File {
public:
    /**
     * Allocate the buffer and open a file.
     *
     * @param pszFile The file, relative to the root of the game files.
     * @param nMode The open mode, one or more of File::Mode.
     * @ghidraAddress NTSC-U/C: 0x0028eb50
     * @ghidraAddress PAL: 0x00298518
     */
    AsyncFile(const char *pszFile, int nMode);

    /**
     * Close the file and release the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x0028ed28
     * @ghidraAddress PAL: 0x002986f0
     */
    ~AsyncFile() override;

    /**
     * Read the `buf_size` setting of the `file` block of the system configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x0028eaf8
     * @ghidraAddress PAL: 0x002984c0
     */
    static void Init();

    /**
     * Shut the file layer down.
     *
     * The body does nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x0028eb48
     * @ghidraAddress PAL: 0x00298510
     */
    static void Terminate();

    /**
     * Read bytes and wait for them.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes.
     * @return The number of bytes read, or 0 after a failure.
     * @ghidraAddress NTSC-U/C: 0x0028ee18
     * @ghidraAddress PAL: 0x002987e0
     */
    int Read(void *pBuffer, int nBytes) override;

    /**
     * Start reading bytes.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes, cut at the end of the file.
     * @return Whether the read started, false after a failure.
     * @ghidraAddress NTSC-U/C: 0x0028ee88
     * @ghidraAddress PAL: 0x00298850
     */
    bool ReadAsync(void *pBuffer, int nBytes) override;

    /**
     * Write bytes into the buffer, writing out each full buffer.
     *
     * @param pBuffer The bytes.
     * @param nBytes The number of bytes.
     * @return nBytes, or 0 after a failure.
     * @ghidraAddress NTSC-U/C: 0x0028f050
     * @ghidraAddress PAL: 0x00298a18
     */
    int Write(const void *pBuffer, int nBytes) override;

    /**
     * Move the position, clamped to the file, and refill the buffer of a file opened to read.
     *
     * @param nOffset The offset.
     * @param nOrigin The origin of the offset. Another value leaves the position unchanged.
     * @return The new position.
     * @ghidraAddress NTSC-U/C: 0x0028f178
     * @ghidraAddress PAL: 0x00298b40
     */
    int Seek(int nOffset, int nOrigin) override;

    /**
     * Report the position.
     *
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x0028f270
     */
    int Tell() override {
        return mTell;
    }

    /**
     * Write out the buffer of a file opened to write, or start refilling the buffer of a file
     * opened to read.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f278
     * @ghidraAddress PAL: 0x00298c40
     */
    void Flush() override;

    /**
     * Report whether the position is at the end.
     *
     * @return Whether the position equals the size.
     * @ghidraAddress NTSC-U/C: 0x0028f340
     */
    bool Eof() override {
        return mTell == mSize;
    }

    /**
     * Report whether an operation failed.
     *
     * @return Whether an operation failed.
     * @ghidraAddress NTSC-U/C: 0x0028f358
     */
    bool Fail() override {
        return mFail != 0;
    }

    /**
     * Report the size.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x0028f360
     */
    int Size() override {
        return mSize;
    }

    /**
     * Report the size before compression.
     *
     * @return The size in bytes, or 0 for a file that is not a `.gz` file opened to read.
     * @ghidraAddress NTSC-U/C: 0x0028f368
     */
    int UncompressedSize() override {
        return mUncompressedSize;
    }

    /**
     * Copy what the buffer has of the read, refilling the buffer when the read needs more.
     *
     * @param pnBytes Receives the number of bytes read so far, except when a refill starts.
     * @return Whether the read finished.
     * @ghidraAddress NTSC-U/C: 0x0028ef00
     * @ghidraAddress PAL: 0x002988c8
     */
    bool ReadDone(int *pnBytes) override;

    /**
     * Open a file, and read the size before compression of a `.gz` file opened to read.
     *
     * @param pszFile The file, relative to the root of the game files.
     * @param nMode The open mode.
     * @ghidraAddress NTSC-U/C: 0x0028ec20
     * @ghidraAddress PAL: 0x002985e8
     */
    void Open(const char *pszFile, int nMode);

    /**
     * Write out the buffer of a file opened to write, close the file, and reset the state.
     *
     * @ghidraAddress NTSC-U/C: 0x0028ed98
     * @ghidraAddress PAL: 0x00298760
     */
    void Close();

    int mMode;             /*!< The open mode. */
    int mFd;               /*!< The descriptor, or negative while the file is not open. */
    int mSize;             /*!< The size in bytes. */
    int mUncompressedSize; /*!< The size before compression of a `.gz` file, or 0. */
    int mTell;             /*!< The position. */
    int mBufferOffset;     /*!< The position in the buffer. */
    int mReadStarted;      /*!< Non-zero once the buffer was first filled. */
    int mFail;             /*!< Non-zero once an operation failed. */
    char *mBuffer;         /*!< The buffer. */
    char *mReadDest;       /*!< Where the read copies its next bytes. */
    int mReadRemaining;    /*!< Bytes the read still copies. */
    int mReadDone;         /*!< Bytes the read copied. */
    String mFilename;      /*!< The device path. */

private:
    /**
     * Close the descriptors of files opened to read until a descriptor is free, while every
     * descriptor is in use.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f370
     * @ghidraAddress PAL: 0x00298d38
     */
    static void FreeDescriptor();

    /**
     * Find the size of the file and open its descriptor.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x0028f438
     * @ghidraAddress PAL: 0x00298e00
     */
    int OpenDescriptor();

    /**
     * Write bytes and wait for them.
     *
     * @param pBuffer The bytes.
     * @param nBytes The number of bytes.
     * @return nBytes.
     * @ghidraAddress NTSC-U/C: 0x0028f528
     * @ghidraAddress PAL: 0x00298ef0
     */
    int WriteDescriptor(const void *pBuffer, int nBytes);

    /**
     * Move the position of the descriptor to mTell and wait for it.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f598
     * @ghidraAddress PAL: 0x00298f60
     */
    void SeekDescriptor();

    /**
     * Start reading bytes, reopening a descriptor that was closed.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0028f608
     * @ghidraAddress PAL: 0x00298fd0
     */
    void ReadDescriptor(void *pBuffer, int nBytes);

    /**
     * Report whether the request of the descriptor finished.
     *
     * @return Whether the request finished, true without a descriptor.
     * @ghidraAddress NTSC-U/C: 0x0028f6b0
     * @ghidraAddress PAL: 0x00299078
     */
    bool DescriptorDone();

    /**
     * Close the descriptor, leaving the file to reopen it on the next read.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f6f0
     * @ghidraAddress PAL: 0x002990b8
     */
    void CloseDescriptor();
};
