#pragma once

#include "utl/Str.h"

/**
 * Base of the streams that read and write binary data.
 *
 * The object is 8 bytes. Values wider than a byte are stored least significant byte first when the
 * stream is little-endian, and are byte-swapped otherwise.
 */
class BinStream {
public:
    /** The origin of Seek(). */
    enum SeekType {
        kSeekBegin = 0,   /*!< From the start of the stream. */
        kSeekCurrent = 1, /*!< From the position. */
        kSeekEnd = 2,     /*!< From the end of the stream. */
    };

    /**
     * Set the byte order.
     *
     * @param littleEndian Whether values are stored least significant byte first.
     * @ghidraAddress 0x10019590
     */
    BinStream(bool littleEndian);

    /**
     * Destroy the stream.
     *
     * @ghidraAddress 0x10011e40
     */
    virtual ~BinStream() {
    }

    /**
     * Read bytes.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     */
    virtual void Read(void *data, int bytes) = 0;

    /**
     * Write bytes.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     */
    virtual void Write(const void *data, int bytes) = 0;

    /** Write out buffered data. */
    virtual void Flush() = 0;

    /**
     * Move the position.
     *
     * @param offset The offset in bytes.
     * @param type The origin of the offset.
     */
    virtual void Seek(int offset, SeekType type) = 0;

    /** @return The position in bytes. */
    virtual int Tell() = 0;

    /** @return Whether the position is at the end. */
    virtual bool Eof() = 0;

    /** @return Whether a transfer failed. */
    virtual bool Fail() = 0;

    /**
     * Do nothing. No stream of the control overrides or calls this, and its purpose is not known.
     *
     * @ghidraAddress 0x10011e30
     */
    virtual void NoOp(int) {
    }

    /**
     * Read a value, swapping its bytes into the host order.
     *
     * @param data Receives the value.
     * @param bytes The size of the value: 2, 4, or 8.
     * @ghidraAddress 0x100195b0
     */
    void ReadEndian(void *data, int bytes);

    /**
     * Write a value, swapping its bytes into the stream order.
     *
     * @param data The value.
     * @param bytes The size of the value: 2, 4, or 8.
     * @ghidraAddress 0x10019750
     */
    void WriteEndian(const void *data, int bytes);

    /**
     * Write text as its length followed by its characters, without a terminator.
     *
     * @param str The text.
     * @return The stream.
     * @ghidraAddress 0x10019900
     */
    BinStream &operator<<(const char *str);

    /**
     * Read text written by operator<<(const char *).
     *
     * @param str Receives the text.
     * @return The stream.
     * @ghidraAddress 0x10019940
     */
    BinStream &operator>>(String &str);

    /**
     * Read text written by operator<<(const char *) into a buffer, and terminate it.
     *
     * @param buffer Receives the text.
     * @param bufSize The size of the buffer, which must exceed the length of the text.
     * @ghidraAddress 0x10019980
     */
    void ReadString(char *buffer, int bufSize);

    BinStream &operator<<(char c) {
        Write(&c, 1);
        return *this;
    }

    BinStream &operator<<(unsigned char c) {
        Write(&c, 1);
        return *this;
    }

    BinStream &operator<<(short s) {
        WriteEndian(&s, sizeof(s));
        return *this;
    }

    BinStream &operator<<(unsigned short s) {
        WriteEndian(&s, sizeof(s));
        return *this;
    }

    BinStream &operator<<(int i) {
        WriteEndian(&i, sizeof(i));
        return *this;
    }

    BinStream &operator<<(unsigned int i) {
        WriteEndian(&i, sizeof(i));
        return *this;
    }

    BinStream &operator<<(float f) {
        WriteEndian(&f, sizeof(f));
        return *this;
    }

    BinStream &operator>>(char &c) {
        Read(&c, 1);
        return *this;
    }

    BinStream &operator>>(unsigned char &c) {
        Read(&c, 1);
        return *this;
    }

    BinStream &operator>>(short &s) {
        ReadEndian(&s, sizeof(s));
        return *this;
    }

    BinStream &operator>>(unsigned short &s) {
        ReadEndian(&s, sizeof(s));
        return *this;
    }

    BinStream &operator>>(int &i) {
        ReadEndian(&i, sizeof(i));
        return *this;
    }

    BinStream &operator>>(unsigned int &i) {
        ReadEndian(&i, sizeof(i));
        return *this;
    }

    BinStream &operator>>(float &f) {
        ReadEndian(&f, sizeof(f));
        return *this;
    }

private:
    bool mLittleEndian; /*!< Whether values are stored least significant byte first. */
};

/**
 * Read text a bank file stores.
 *
 * Text is read as operator>>(String &) writes it, unless gReadTextStrings selects terminated
 * text, which is read one byte at a time.
 *
 * @param bs The stream.
 * @param str Receives the text.
 * @return The stream.
 * @ghidraAddress 0x10011760
 */
BinStream &ReadTextString(BinStream &bs, String &str);
