#pragma once

#include "utl/PrnStream.h"

/**
 * The shared terminator of every empty String. No String frees it.
 *
 * @ghidraAddress 0x10039400
 */
extern char *gStringEmptyBuffer;

/**
 * Format text with printf() conventions into a static buffer. The text is cut to 1023 characters,
 * and the next call replaces it.
 *
 * @param format The format.
 * @return The text.
 * @ghidraAddress 0x10010cf0
 */
char *FormatString(const char *format, ...);

/**
 * Text that grows as it is printed to.
 *
 * The RTTI records the class as deriving from PrnStream. The object is 0x14 bytes. The buffer
 * comes from the pool allocator and is replaced, not grown, by every change. An empty String
 * points at gStringEmptyBuffer. The length is named by the assertion text of the source; the other
 * member names, and the names of the members compiled inline (c_str(), size(), the default
 * constructor), are inferred.
 */
class String : public PrnStream {
public:
    /** The result of a search that finds nothing. */
    enum { npos = -1 };

    String() : mLen(0), mBuffer(gStringEmptyBuffer) {
    }

    /**
     * Copy text. A null pointer makes an empty String.
     *
     * @param str The text, or null.
     * @ghidraAddress 0x10010d20
     */
    String(const char *str);

    /**
     * Copy another String.
     *
     * @param other The String.
     * @ghidraAddress 0x10010de0
     */
    String(const String &other);

    /**
     * Free the buffer.
     *
     * @ghidraAddress 0x10010ea0
     */
    virtual ~String();

    /**
     * Append printed text.
     *
     * @param str The text.
     * @ghidraAddress 0x10010f00
     */
    virtual void Print(const char *str);

    /**
     * Append another String.
     *
     * @param other The String.
     * @return This String.
     * @ghidraAddress 0x10010f10
     */
    String &operator+=(const String &other);

    /**
     * Append text. A null pointer appends nothing.
     *
     * @param str The text, or null.
     * @return This String.
     * @ghidraAddress 0x10010fe0
     */
    String &operator+=(const char *str);

    /**
     * Replace the text. A null pointer empties the String.
     *
     * @param str The text, or null.
     * @return This String.
     * @ghidraAddress 0x100110b0
     */
    String &operator=(const char *str);

    /**
     * Replace the text with a copy of another String.
     *
     * @param other The String.
     * @return This String.
     * @ghidraAddress 0x10011140
     */
    String &operator=(const String &other);

    /**
     * Compare with text. A null pointer is unequal to every String.
     *
     * @param str The text, or null.
     * @return Whether the texts differ.
     * @ghidraAddress 0x100111f0
     */
    bool operator!=(const char *str) const;

    /**
     * Compare with another String.
     *
     * @param other The String.
     * @return Whether the texts differ.
     * @ghidraAddress 0x10011250
     */
    bool operator!=(const String &other) const;

    /**
     * Compare with text. A null pointer is unequal to every String.
     *
     * @param str The text, or null.
     * @return Whether the texts are equal.
     * @ghidraAddress 0x100112b0
     */
    bool operator==(const char *str) const;

    /**
     * Compare with another String.
     *
     * @param other The String.
     * @return Whether the texts are equal.
     * @ghidraAddress 0x10011310
     */
    bool operator==(const String &other) const;

    /**
     * Order by the bytes of the text.
     *
     * @param other The String.
     * @return Whether this text sorts first.
     * @ghidraAddress 0x10011370
     */
    bool operator<(const String &other) const;

    /**
     * Replace the text with a length of zero bytes.
     *
     * @param length The new length.
     * @ghidraAddress 0x100113d0
     */
    void Resize(unsigned int length);

    /**
     * Find the first occurrence of text.
     *
     * @param str The text.
     * @return The position of the occurrence, or npos.
     * @ghidraAddress 0x10011430
     */
    int Find(const char *str) const;

    /**
     * Find the last occurrence of a character.
     *
     * @param c The character.
     * @return The position of the character, or npos.
     * @ghidraAddress 0x10011460
     */
    int FindLast(char c) const;

    /**
     * Copy the text from a position to the end.
     *
     * @param pos The first character.
     * @return The copy.
     * @ghidraAddress 0x10011490
     */
    String Substring(unsigned int pos) const;

    /**
     * Copy part of the text. A count that passes the end copies to the end.
     *
     * @param pos The first character.
     * @param count The number of characters.
     * @return The copy.
     * @ghidraAddress 0x100114f0
     */
    String Substring(unsigned int pos, unsigned int count) const;

    /**
     * Replace part of the text with another String.
     *
     * @param pos The first character replaced.
     * @param count The number of characters replaced. A count that passes the end is cut to it.
     * @param str The new text.
     * @return This String.
     * @ghidraAddress 0x10011610
     */
    String &Replace(unsigned int pos, unsigned int count, const String &str);

    /**
     * Empty the String.
     *
     * @return This String.
     * @ghidraAddress 0x10011720
     */
    String &Clear();

    /** @return The terminated text. */
    const char *c_str() const {
        return mBuffer;
    }

    /** @return The terminated text. */
    operator const char *() const {
        return c_str();
    }

    /** @return The number of characters before the terminator. */
    unsigned int size() const {
        return mLen;
    }

private:
    friend class BinStream;

    unsigned int mLen;      /*!< Characters before the terminator. */
    unsigned int mCapacity; /*!< Bytes allocated for mBuffer. Unset while the String is empty. */
    char *mBuffer;          /*!< The terminated text. */
};
