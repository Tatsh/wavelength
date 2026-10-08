#pragma once

#include "os/binstream.h"
#include "os/file.h"
#include "os/string.h"

/**
 * BinStream over a file read into a buffer in the background.
 *
 * The RTTI includes the class name and records BinStream as the base. The constructor opens the
 * file and starts reading all of it into a buffer from the pool, billed to the tag
 * "AsyncStream buf". Only the members its callers here use are declared.
 */
class AsyncStream : public BinStream {
public:
    /**
     * Open a file and start reading it.
     *
     * @param pszFile The file.
     * @param bLittleEndian Whether values are stored in the console's byte order.
     * @ghidraAddress NTSC-U/C: 0x00293ac0
     * @ghidraAddress PAL: 0x0029d488
     */
    AsyncStream(const char *pszFile, bool bLittleEndian);

    /**
     * Release the buffer and the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00293ba0
     * @ghidraAddress PAL: 0x0029d568
     */
    ~AsyncStream() override;

    /**
     * Report whether enough of the file has been read.
     *
     * Once the whole file has been read, the file is closed and every later call reports true.
     *
     * @param nBytes The bytes needed.
     * @return Whether the read has finished or at least nBytes have arrived.
     * @ghidraAddress NTSC-U/C: 0x00293c38
     * @ghidraAddress PAL: 0x0029d5f0
     */
    bool Ready(int nBytes);

    /**
     * Read bytes, waiting for them to arrive.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x00293cc8
     * @ghidraAddress PAL: 0x0029d680
     */
    void Read(void *pData, int nBytes) override;

    /**
     * Write bytes.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x00293d70
     * @ghidraAddress PAL: 0x0029d728
     */
    void Write(const void *pData, int nBytes) override;

    /**
     * Do nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x003a8a70
     */
    void Flush() override {
    }

    /**
     * Move the position.
     *
     * @param nOffset The offset in bytes.
     * @param eFrom The origin of the offset.
     * @ghidraAddress NTSC-U/C: 0x00293e18
     */
    void Seek(int nOffset, SeekType eFrom) override;

    /**
     * Report the position.
     *
     * @return The position in bytes.
     * @ghidraAddress NTSC-U/C: 0x003a8a78
     */
    int Tell() override {
        return mPosition;
    }

    /**
     * Report the file.
     *
     * GfxManager::FindPreload() reads the name in place, and the image has no accessor.
     *
     * @return The name of the file.
     */
    const char *GetFileName() const {
        return mFileName.c_str();
    }

    /**
     * Report whether the position is at the end of the file.
     *
     * @return Whether no byte is left.
     * @ghidraAddress NTSC-U/C: 0x003a8a80
     */
    bool Eof() override {
        return mPosition == mSize;
    }

    /**
     * Report whether a transfer failed.
     *
     * @return Whether a transfer failed.
     * @ghidraAddress NTSC-U/C: 0x003a8a98
     */
    bool Fail() override {
        return mFail != 0;
    }

private:
    String mFileName; /*!< The file. */
    int mFail;        /*!< Whether a transfer failed. */
    int mPosition;    /*!< The position in the buffer. */
    int mSize;        /*!< The size of the file in bytes, or -1. */
    char *mBuffer;    /*!< The contents of the file. */
    File *mFile;      /*!< The file while it is read, or null. */
};
