#pragma once

#include <list>
#include <vector>

#include "os/string.h"

/**
 * Stream of raw bytes, the base of the memory, file, and network streams.
 *
 * The RTTI includes the class name. One data word precedes the vptr at `+0x04`. Read() and Write()
 * are pure in this class. Only the members its callers here use are declared.
 */
class BinStream {
public:
    /**
     * Construct a stream.
     *
     * @param bLittleEndian Whether values are stored in the console's byte order.
     * @ghidraAddress NTSC-U/C: 0x00293f10
     * @ghidraAddress PAL: 0x0029d8c8
     */
    explicit BinStream(bool bLittleEndian) : mLittleEndian(bLittleEndian) {
    }

    /**
     * Release the stream.
     *
     * @ghidraAddress NTSC-U/C: 0x00339f10
     * @ghidraAddress PAL: 0x0041a390
     */
    virtual ~BinStream() {
    }

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
     * Receive a named mark in the data, such as the end of a chunk.
     *
     * The base ignores the mark. ChunkStream ends its block at a mark of g_szChunkMark.
     *
     * @param pszMark The mark.
     * @ghidraAddress NTSC-U/C: 0x00339f40
     * @ghidraAddress PAL: 0x004177d8
     */
    virtual void Mark(const char *pszMark) {
        (void)pszMark;
    }

    /**
     * Read a length-prefixed string into a buffer and terminate it.
     *
     * The length is not checked against the size of the buffer.
     *
     * @param pszBuffer Receives the text. The caller provides room for it.
     * @param nSize The size of the buffer.
     * @ghidraAddress NTSC-U/C: 0x00294468
     * @ghidraAddress PAL: 0x0029e098
     */
    void ReadString(char *pszBuffer, int nSize);

    /**
     * Read a length-prefixed string.
     *
     * @param text Receives the text.
     * @ghidraAddress NTSC-U/C: 0x00294408
     * @ghidraAddress PAL: 0x0029e040
     */
    void ReadString(String &text);

    /**
     * Write a string with its length first.
     *
     * @param pszText The text.
     * @return This stream.
     * @ghidraAddress NTSC-U/C: 0x00294390
     * @ghidraAddress PAL: 0x0029dfd8
     */
    BinStream &WriteString(const char *pszText);

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

private:
    int mLittleEndian; /*!< Whether values are stored in the console's byte order. */
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
 * @ghidraAddress PAL: 0x002a9528
 */
BinStream &operator>>(BinStream &stream, String &text);

/**
 * Write a vector as its element count followed by each element.
 *
 * Each element type has its own instance, one address each.
 *
 * @tparam T The type of the elements.
 * @param stream The stream to write to.
 * @param elements The elements.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003875b8
 * @ghidraAddress PAL: 0x003f5cc0
 * @ghidraAddress NTSC-U/C: 0x00389208
 * @ghidraAddress PAL: 0x003f7910
 * @ghidraAddress NTSC-U/C: 0x00385520
 * @ghidraAddress PAL: 0x003f3c28
 * @ghidraAddress NTSC-U/C: 0x00385608
 * @ghidraAddress PAL: 0x003f3d10
 * @ghidraAddress NTSC-U/C: 0x003876b0
 * @ghidraAddress PAL: 0x003f5db8
 * @ghidraAddress NTSC-U/C: 0x003921d0
 * @ghidraAddress PAL: 0x004008d8
 * @ghidraAddress NTSC-U/C: 0x0039d1d8
 * @ghidraAddress PAL: 0x0040bf00
 * @ghidraAddress NTSC-U/C: 0x0039d7a0
 * @ghidraAddress PAL: 0x0040c4c8
 * @ghidraAddress NTSC-U/C: 0x0038c050
 * @ghidraAddress PAL: 0x003fa758
 * @ghidraAddress NTSC-U/C: 0x0038c1e0
 * @ghidraAddress PAL: 0x003fa8e8
 * @ghidraAddress NTSC-U/C: 0x0038c3a0
 * @ghidraAddress PAL: 0x003faaa8
 * @ghidraAddress NTSC-U/C: 0x003a4a70
 * @ghidraAddress PAL: 0x00413750
 * @ghidraAddress NTSC-U/C: 0x00383608
 * @ghidraAddress PAL: 0x003f1d10
 * @ghidraAddress NTSC-U/C: 0x003893e0
 * @ghidraAddress PAL: 0x003f7ae8
 * @ghidraAddress NTSC-U/C: 0x003894b0
 * @ghidraAddress PAL: 0x003f7bb8
 * @ghidraAddress NTSC-U/C: 0x00393dd0
 * @ghidraAddress PAL: 0x004024d8
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const std::vector<T> &elements) {
    const int nSize = static_cast<int>(elements.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const T &element : elements) {
        stream << element;
    }
    return stream;
}

/**
 * Read a vector the vector writer wrote.
 *
 * The vector is resized to the count with default elements, which are then read in place. Each
 * element type has its own instance, one address each.
 *
 * @tparam T The type of the elements.
 * @param stream The stream to read from.
 * @param elements Receives the elements.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00387e60
 * @ghidraAddress PAL: 0x003f6568
 * @ghidraAddress NTSC-U/C: 0x00389558
 * @ghidraAddress PAL: 0x003f7c60
 * @ghidraAddress NTSC-U/C: 0x00385ba8
 * @ghidraAddress PAL: 0x003f42b0
 * @ghidraAddress NTSC-U/C: 0x00386058
 * @ghidraAddress PAL: 0x003f4760
 * @ghidraAddress NTSC-U/C: 0x00388398
 * @ghidraAddress PAL: 0x003f6aa0
 * @ghidraAddress NTSC-U/C: 0x003922d0
 * @ghidraAddress PAL: 0x004009d8
 * @ghidraAddress NTSC-U/C: 0x00389670
 * @ghidraAddress PAL: 0x003f7d78
 * @ghidraAddress NTSC-U/C: 0x00389768
 * @ghidraAddress PAL: 0x003f7e70
 * @ghidraAddress NTSC-U/C: 0x0038a3e8
 * @ghidraAddress PAL: 0x003f8af0
 * @ghidraAddress NTSC-U/C: 0x0038a4a0
 * @ghidraAddress PAL: 0x003f8ba8
 * @ghidraAddress NTSC-U/C: 0x0038c9c0
 * @ghidraAddress PAL: 0x003fb0c8
 * @ghidraAddress NTSC-U/C: 0x0038d1b8
 * @ghidraAddress PAL: 0x003fb8c0
 * @ghidraAddress NTSC-U/C: 0x0038d9c0
 * @ghidraAddress PAL: 0x003fc0c8
 * @ghidraAddress NTSC-U/C: 0x00393f20
 * @ghidraAddress PAL: 0x00402610
 * @ghidraAddress NTSC-U/C: 0x0039d5e8
 * @ghidraAddress PAL: 0x0040c310
 * @ghidraAddress NTSC-U/C: 0x0039dbb0
 * @ghidraAddress PAL: 0x0040c8d8
 * @ghidraAddress NTSC-U/C: 0x003a4f20
 * @ghidraAddress PAL: 0x00413c00
 * @ghidraAddress NTSC-U/C: 0x00383740
 * @ghidraAddress PAL: 0x003f1e48
 */
template <typename T>
BinStream &operator>>(BinStream &stream, std::vector<T> &elements) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    elements.resize(nSize, T());
    for (T &element : elements) {
        stream >> element;
    }
    return stream;
}

/**
 * Write a list as its element count followed by each element.
 *
 * Each element type has its own instance, one address each.
 *
 * @tparam T The type of the elements.
 * @param stream The stream to write to.
 * @param elements The elements.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0039a878
 * @ghidraAddress PAL: 0x00409100
 */
template <typename T>
BinStream &operator<<(BinStream &stream, const std::list<T> &elements) {
    const int nSize = static_cast<int>(elements.size());
    stream.WriteEndian(&nSize, sizeof(nSize));
    for (const T &element : elements) {
        stream << element;
    }
    return stream;
}

/**
 * Read a list the list writer wrote.
 *
 * The list is resized to the count with default elements, which are then read in place. Each
 * element type has its own instance, one address each.
 *
 * @tparam T The type of the elements.
 * @param stream The stream to read from.
 * @param elements Receives the elements.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0039aaf8
 * @ghidraAddress PAL: 0x00409380
 */
template <typename T>
BinStream &operator>>(BinStream &stream, std::list<T> &elements) {
    int nSize;
    stream.ReadEndian(&nSize, sizeof(nSize));
    elements.resize(nSize, T());
    for (T &element : elements) {
        stream >> element;
    }
    return stream;
}
