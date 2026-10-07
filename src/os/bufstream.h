#pragma once

#include "os/binstream.h"

/**
 * BinStream over a buffer the caller provides.
 *
 * The RTTI includes the class name and records BinStream as the base. A read past the end of the
 * buffer is shortened to the bytes left and marks the stream failed. Only the members its callers
 * here use are declared.
 */
class BufStream : public BinStream {
public:
    /**
     * Construct a stream at the start of a buffer.
     *
     * A null buffer starts the stream failed.
     *
     * @param pBuffer The buffer. The caller retains it.
     * @param nSize The size of the buffer in bytes.
     * @param bLittleEndian Whether values are stored in the console's byte order.
     * @ghidraAddress NTSC-U/C: 0x002944c8
     * @ghidraAddress PAL: 0x0029e0f0
     */
    BufStream(char *pBuffer, int nSize, bool bLittleEndian);

    /**
     * Release the stream. The buffer is not released.
     *
     * @ghidraAddress NTSC-U/C: 0x003a8ae0
     */
    ~BufStream() override;

    /**
     * Read bytes.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x00294530
     * @ghidraAddress PAL: 0x0029e158
     */
    void Read(void *pData, int nBytes) override;

    /**
     * Write bytes.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x002945a0
     */
    void Write(const void *pData, int nBytes) override;

    /**
     * Do nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x003a8b60
     */
    void Flush() override {
    }

    /**
     * Move the position, failing the stream when it would leave the buffer.
     *
     * @param nOffset The offset in bytes.
     * @param eFrom The origin of the offset.
     * @ghidraAddress NTSC-U/C: 0x00294610
     */
    void Seek(int nOffset, SeekType eFrom) override;

    /**
     * Report the position.
     *
     * @return The position in bytes.
     * @ghidraAddress NTSC-U/C: 0x003a8b68
     */
    int Tell() override {
        return mPosition;
    }

    /**
     * Report whether the position is at the end of the buffer.
     *
     * @return Whether no byte is left.
     * @ghidraAddress NTSC-U/C: 0x003a8b70
     */
    bool Eof() override {
        return mPosition == mSize;
    }

    /**
     * Report whether a transfer ran past the end.
     *
     * @return Whether a transfer failed.
     * @ghidraAddress NTSC-U/C: 0x003a8b88
     */
    bool Fail() override {
        return mFail != 0;
    }

private:
    char *mBuffer; /*!< The buffer. */
    int mFail;     /*!< Whether a transfer ran past the end, or the buffer is null. */
    int mPosition; /*!< The position in the buffer. */
    int mSize;     /*!< The size of the buffer in bytes. */
};
