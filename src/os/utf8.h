#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <memory>

#include "os/string.h"

/**
 * Report the bytes a text of the console's character set takes in UTF-8, the terminator included.
 *
 * A character above 127 takes two bytes and any other character one. The name is inferred.
 *
 * @param pszText The text.
 * @return The size in bytes.
 * @ghidraAddress PAL: 0x002aa2b0
 */
int Utf8Size(const char *pszText);

/**
 * Write a text of the console's character set in UTF-8, with a terminator.
 *
 * A character above 127 becomes a two-byte sequence. The name is inferred.
 *
 * @param pszOut The destination, of at least Utf8Size() bytes.
 * @param pszText The text.
 * @param nSize The size of the destination. The routine does not read it.
 * @ghidraAddress PAL: 0x002aa320
 */
void Utf8Encode(char *pszOut, const char *pszText, int nSize);

/**
 * Report the bytes a UTF-8 text takes once Utf8Decode() converts it, the terminator included.
 *
 * Each sequence counts one byte. A lead byte from 0xc0 to 0xcf starts a two-byte sequence, from
 * 0xe0 to 0xef a three-byte sequence, and from 0xf0 to 0xff a four-byte sequence. Any other byte
 * stands alone. The name is inferred.
 *
 * @param pszUtf8 The UTF-8 text.
 * @return The size in bytes.
 * @ghidraAddress PAL: 0x002aa3d0
 */
int Utf8DecodedSize(const char *pszUtf8);

/**
 * Write a UTF-8 text in the console's character set, with a terminator.
 *
 * A lead byte from 0xc0 to 0xdf starts a two-byte sequence, which becomes one character from its
 * low eight bits. A lead byte from 0xe0 to 0xef skips three bytes and any other byte from 0x80
 * skips four, and each skip advances the destination without writing it. The name is inferred.
 *
 * @param pszOut The destination, of at least Utf8DecodedSize() bytes.
 * @param pszUtf8 The UTF-8 text.
 * @param nSize The size of the destination. The routine does not read it.
 * @ghidraAddress PAL: 0x002aa488
 */
void Utf8Decode(char *pszOut, const char *pszUtf8, int nSize);

/**
 * Make a String of a text in UTF-8, through a buffer from the heap.
 *
 * Inline at each call.
 *
 * @param pszText The text in the console's character set.
 * @return The UTF-8 text.
 */
inline String Utf8String(const char *pszText) {
    const int nSize = Utf8Size(pszText);
    auto pszBuffer = std::make_unique<char[]>(nSize);
    Utf8Encode(pszBuffer.get(), pszText, nSize);
    String text(pszBuffer.get());
    return text;
}

/**
 * Make a String of a UTF-8 text in the console's character set, through a buffer from the heap.
 *
 * Inline at each call.
 *
 * @param pszUtf8 The UTF-8 text.
 * @return The text in the console's character set.
 */
inline String Utf8DecodedString(const char *pszUtf8) {
    const int nSize = Utf8DecodedSize(pszUtf8);
    auto pszBuffer = std::make_unique<char[]>(nSize);
    Utf8Decode(pszBuffer.get(), pszUtf8, nSize);
    String text(pszBuffer.get());
    return text;
}

#endif
