#pragma once

#include "os/file.h"
#include "os/string.h"

/**
 * Read-only file of the main archive, read through the disc cache.
 *
 * The RTTI includes the class name and records File as the base.
 */
class ArkFile : public File {
public:
    /**
     * Find a file in the main archive.
     *
     * @param pszFile The file, relative to the root of the game files.
     * @param nMode The open mode. A mode that writes fails.
     * @ghidraAddress NTSC-U/C: 0x0028e670
     * @ghidraAddress PAL: 0x00298038
     */
    ArkFile(const char *pszFile, int nMode);

    /**
     * Drop the reads still waiting for the disc.
     *
     * @ghidraAddress NTSC-U/C: 0x0028e720
     * @ghidraAddress PAL: 0x002980e8
     */
    ~ArkFile() override;

    /**
     * Express a path relative to the root of the game files, as the archive records it.
     *
     * @param pszPath The path.
     * @return A shared buffer the next call replaces.
     * @ghidraAddress NTSC-U/C: 0x0028e5f0
     * @ghidraAddress PAL: 0x00297fb8
     */
    static const char *ArchivePath(const char *pszPath);

    /**
     * Read bytes and wait for them.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes.
     * @return The number of bytes read, or 0 when the read did not start.
     * @ghidraAddress NTSC-U/C: 0x0028e798
     * @ghidraAddress PAL: 0x00298160
     */
    int Read(void *pBuffer, int nBytes) override;

    /**
     * Start reading bytes, one task for each block of the disc they cover.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes, cut at the end of the file.
     * @return Whether the read started, false at the end or while a read is waiting.
     * @ghidraAddress NTSC-U/C: 0x0028e808
     * @ghidraAddress PAL: 0x002981d0
     */
    bool ReadAsync(void *pBuffer, int nBytes) override;

    /**
     * Report that an archive file cannot be written.
     *
     * @param pBuffer The bytes.
     * @param nBytes The number of bytes.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x0028e9c8
     * @ghidraAddress PAL: 0x00298390
     */
    int Write(const void *pBuffer, int nBytes) override;

    /**
     * Move the position.
     *
     * @param nOffset The offset.
     * @param nOrigin The origin of the offset. Another value leaves the position unchanged.
     * @return The new position.
     * @ghidraAddress NTSC-U/C: 0x0028e9f0
     */
    int Seek(int nOffset, int nOrigin) override;

    /**
     * Report the position.
     *
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x0028ea40
     */
    int Tell() override {
        return mTell;
    }

    /**
     * Do nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x003a7f00
     */
    void Flush() override {
    }

    /**
     * Report whether the position is at the end.
     *
     * @return Whether the position equals the size.
     * @ghidraAddress NTSC-U/C: 0x0028ea48
     */
    bool Eof() override {
        return mTell == mSize;
    }

    /**
     * Report whether the file was not found.
     *
     * @return Whether the file failed.
     * @ghidraAddress NTSC-U/C: 0x0028ea60
     */
    bool Fail() override {
        return mFail != 0;
    }

    /**
     * Report the size.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x0028ea70
     */
    int Size() override {
        return mSize;
    }

    /**
     * Report the size before compression.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x0028ea78
     */
    int UncompressedSize() override {
        return mUncompressedSize;
    }

    /**
     * Advance the disc cache and report whether every task of the read finished.
     *
     * @param pnBytes Receives the number of bytes read once the read finished.
     * @return Whether the read finished.
     * @ghidraAddress NTSC-U/C: 0x0028eaa8
     * @ghidraAddress PAL: 0x00298470
     */
    bool ReadDone(int *pnBytes) override;

    /**
     * Count a finished task of the read.
     *
     * @param nBytes The bytes the task copied.
     * @ghidraAddress NTSC-U/C: 0x0028ea80
     * @ghidraAddress PAL: 0x00298448
     */
    void TaskDone(int nBytes);

    long long mArkOffset;  /*!< The byte of the disc the file starts at. */
    int mSize;             /*!< The size in bytes. */
    int mUncompressedSize; /*!< The size before compression. */
    int mNumOutstanding;   /*!< Tasks of the read still waiting. */
    int mBytesRead;        /*!< Bytes of the read the finished tasks copied. */
    int mTell;             /*!< The position. */
    int mFail;             /*!< Non-zero when the file was not found. */
    String mFilename;      /*!< Constructed empty and never set. */
};
