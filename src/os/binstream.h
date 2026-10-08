#pragma once

#include "os/string.h"

/**
 * Stream of raw bytes, the base of the memory, file, and network streams.
 *
 * The RTTI includes the class name. One data word precedes the vptr at `+0x04`. Read() and Write()
 * are pure in this class. Only the members its callers here use are declared.
 */
class BinStream {
public:
    /** Release the stream. */
    virtual ~BinStream();

    /**
     * Read bytes.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     */
    virtual void Read(void *pData, int nBytes) = 0;

    /**
     * Write bytes.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     */
    virtual void Write(const void *pData, int nBytes) = 0;

    /** Origins of Seek(). */
    enum SeekType {
        kSeekBegin = 0,   /*!< From the start of the stream. */
        kSeekCurrent = 1, /*!< From the current position. */
        kSeekEnd = 2,     /*!< From the end of the stream. */
    };

    /** Write out buffered data. */
    virtual void Flush() = 0;

    /**
     * Move the position.
     *
     * @param nOffset The offset in bytes.
     * @param eFrom The origin of the offset.
     */
    virtual void Seek(int nOffset, SeekType eFrom) = 0;

    /**
     * Report the position.
     *
     * @return The position in bytes.
     */
    virtual int Tell() = 0;

    /**
     * Report whether the position is at the end.
     *
     * @return Whether no byte is left.
     */
    virtual bool Eof() = 0;

    /**
     * Report whether a transfer failed.
     *
     * @return Whether a transfer failed.
     */
    virtual bool Fail() = 0;

    /**
     * Read a value of 2, 4, 8, or 16 bytes, reversing its bytes unless the stream is little endian.
     *
     * @param pData The destination.
     * @param nBytes The size of the value.
     * @ghidraAddress NTSC-U/C: 0x00293f28
     * @ghidraAddress PAL: 0x0029db90
     */
    void ReadEndian(void *pData, int nBytes);

    /**
     * Write a value of 2, 4, 8, or 16 bytes, reversing its bytes unless the stream is little
     * endian.
     *
     * @param pData The source.
     * @param nBytes The size of the value.
     * @ghidraAddress NTSC-U/C: 0x00294158
     * @ghidraAddress PAL: 0x0029ddb0
     */
    void WriteEndian(const void *pData, int nBytes);

    /**
     * Write text as its length followed by its characters, without the terminator.
     *
     * @param pszText The text.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x00294390
     * @ghidraAddress PAL: 0x0029dfd8
     */
    BinStream &WriteString(const char *pszText);

    /**
     * Read text WriteString() wrote into a string.
     *
     * @param text Receives the text.
     * @ghidraAddress NTSC-U/C: 0x00294408
     */
    void ReadString(String &text);

    /**
     * Read text WriteString() wrote into a buffer and terminate it.
     *
     * @param pszBuffer Receives the text.
     * @param nBufferSize The size of the buffer. The routine does not check it.
     * @ghidraAddress NTSC-U/C: 0x00294468
     */
    void ReadString(char *pszBuffer, int nBufferSize);

private:
    bool mLittleEndian; /*!< Whether values are stored in the console's byte order. */
};

/**
 * Read text into a string.
 *
 * In the text mode of the string streams the text is read up to its terminator. Otherwise it is
 * read as BinStream::WriteString() wrote it.
 *
 * @param stream The stream to read from.
 * @param text Receives the text.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0029f868
 */
BinStream &operator>>(BinStream &stream, String &text);
