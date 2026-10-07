#pragma once

#include "os/prnstream.h"

/**
 * Shared terminated buffer every empty String points at.
 *
 * @ghidraAddress NTSC-U/C: 0x003b24d0
 */
extern char *g_szStringEmptyBuffer;

/**
 * Growable text buffer that is also a PrnStream.
 *
 * The RTTI includes the class name and records PrnStream as the base. An empty string points
 * mBuffer at g_szStringEmptyBuffer rather than at an allocation of its own.
 */
class String : public PrnStream {
public:
    /**
     * Construct an empty string.
     *
     * The constructor has no out-of-line copy, and it does not set mCapacity.
     */
    String() : mLength(0), mBuffer(g_szStringEmptyBuffer) {
    }

    /**
     * Construct a copy of a C string.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0029e9b0
     * @ghidraAddress PAL: 0x002a8670
     */
    explicit String(const char *pszText);

    /**
     * Copy another string.
     *
     * @param other The string.
     * @ghidraAddress NTSC-U/C: 0x0029ea40
     * @ghidraAddress PAL: 0x002a8700
     */
    String(const String &other);

    /**
     * Release the text.
     *
     * @ghidraAddress NTSC-U/C: 0x0029ead8
     * @ghidraAddress PAL: 0x002a8798
     */
    ~String() override;

    /**
     * Append text.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0029eb48
     * @ghidraAddress PAL: 0x002a8808
     */
    void Print(const char *pszText) override;

    /**
     * Replace the text with a copy of another string's text.
     *
     * The shared empty buffer is shared again rather than copied.
     *
     * @param other The other string.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029ef78
     * @ghidraAddress PAL: 0x002a8c38
     */
    String &operator=(const String &other);

    /**
     * Replace the text with a copy of a C string.
     *
     * @param pszText The text. A null pointer empties the string.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029eed8
     * @ghidraAddress PAL: 0x002a8b98
     */
    String &operator=(const char *pszText);

    /**
     * Release the text and leave the string empty.
     *
     * The name is inferred.
     *
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029f528
     * @ghidraAddress PAL: 0x002a91e8
     */
    String &Clear();

    /**
     * Report one character.
     *
     * @param nIndex The character's position.
     * @return The character.
     * @ghidraAddress NTSC-U/C: 0x0029f020
     * @ghidraAddress PAL: 0x002a8ce0
     */
    char operator[](int nIndex) const;

    /**
     * Replace a run of characters with another string's text.
     *
     * @param nPos The first character of the run.
     * @param nCount The length of the run.
     * @param text The replacement.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029f438
     * @ghidraAddress PAL: 0x002a90f8
     */
    String &Replace(int nPos, int nCount, const String &text);

    /**
     * Cut the text to a length.
     *
     * The shared empty buffer is left unchanged.
     *
     * @param nLength The new length.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029f580
     * @ghidraAddress PAL: 0x002a9240
     */
    String &Truncate(int nLength);

    /**
     * Remove a run of characters.
     *
     * A run that reaches the end of the text truncates the text at nPos.
     *
     * @param nPos The first character of the run.
     * @param nCount The length of the run.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029f5a8
     * @ghidraAddress PAL: 0x002a9268
     */
    String &Erase(int nPos, int nCount);

    /**
     * Append another string's text.
     *
     * @param other The other string.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029ec88
     * @ghidraAddress PAL: 0x002a8948
     */
    String &operator+=(const String &other);

    /**
     * Append a C string.
     *
     * @param pszText The text.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029ed58
     * @ghidraAddress PAL: 0x002a8a18
     */
    String &operator+=(const char *pszText);

    /**
     * Compare a run of characters with a C string.
     *
     * @param nPos The first character of the run.
     * @param nCount The length of the run.
     * @param pszText The text to compare with.
     * @return 0 when the run equals the text, otherwise the sign of the difference.
     * @ghidraAddress NTSC-U/C: 0x0029f2f0
     * @ghidraAddress PAL: 0x002a8fb0
     */
    int Compare(int nPos, int nCount, const char *pszText) const;

    /** The position Find() reports when the text does not occur. */
    static constexpr int npos = -1;

    /**
     * Find the first occurrence of a C string in the text.
     *
     * @param pszText The text to find.
     * @return The position of the occurrence, or npos.
     * @ghidraAddress NTSC-U/C: 0x0029f250
     * @ghidraAddress PAL: 0x002a8f10
     */
    int Find(const char *pszText) const;

    /**
     * Copy the characters from a position to the end.
     *
     * @param nPos The first character copied.
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0029f328
     * @ghidraAddress PAL: 0x002a8fe8
     */
    String Substring(int nPos) const;

    /**
     * Copy a run of characters, or the characters from a position to the end when the run
     * reaches the end.
     *
     * @param nPos The first character copied.
     * @param nCount The length of the run.
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0029f358
     * @ghidraAddress PAL: 0x002a9018
     */
    String Substring(int nPos, int nCount) const;

    /**
     * Insert copies of one character.
     *
     * @param nPos The position the copies go before.
     * @param nCount The number of copies.
     * @param ch The character.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029f630
     * @ghidraAddress PAL: 0x002a92f0
     */
    String &Insert(int nPos, int nCount, char ch);

    /**
     * Report whether the text sorts before another string's text.
     *
     * @param other The other string.
     * @return Whether `strcmp()` places this text first.
     * @ghidraAddress NTSC-U/C: 0x0029f0f0
     * @ghidraAddress PAL: 0x002a8db0
     */
    bool operator<(const String &other) const;

    /**
     * Report whether the text equals a C string.
     *
     * @param pszText The C string, or null.
     * @return Whether `strcmp()` reports the texts equal, or false for a null C string.
     * @ghidraAddress NTSC-U/C: 0x0029f090
     * @ghidraAddress PAL: 0x002a8d50
     */
    bool operator==(const char *pszText) const;

    /**
     * Report whether the text differs from a C string.
     *
     * @param pszText The C string, or null.
     * @return Whether `strcmp()` reports the texts different, or true for a null C string.
     * @ghidraAddress NTSC-U/C: 0x0029f030
     * @ghidraAddress PAL: 0x002a8cf0
     */
    bool operator!=(const char *pszText) const;

    /**
     * Report a copy of the string with text appended.
     *
     * @param pszText The text to append.
     * @return The new string.
     * @ghidraAddress NTSC-U/C: 0x0029eb68
     * @ghidraAddress PAL: 0x002a8828
     */
    String operator+(const char *pszText) const;

    /**
     * Report the text.
     *
     * @return The terminated text, which the string retains.
     */
    const char *c_str() const {
        return mBuffer;
    }

    int mLength;   /*!< Characters before the terminator. */
    int mCapacity; /*!< Bytes allocated for mBuffer. */
    char *mBuffer; /*!< The terminated text. */
};

/**
 * Format text into one shared buffer and return it.
 *
 * The result remains valid only until the next call.
 *
 * @param pszFormat A printf-style format string.
 * @return The shared buffer.
 * @ghidraAddress NTSC-U/C: 0x0029e948
 * @ghidraAddress PAL: 0x002a8608
 */
const char *FormatString(const char *pszFormat, ...);
