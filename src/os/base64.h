#pragma once

/**
 * Encode bytes as text, four characters for each three bytes.
 *
 * The alphabet is the digits, the upper and lower case letters, `.`, and a space, in that order.
 * The first character gives the number of bytes in the last group, 0 for a whole group. The
 * last group is always encoded whole, with the bytes after the input. The name is inferred.
 *
 * @param pszOut Receives the text, which is not terminated.
 * @param pnOutLength Receives the number of characters.
 * @param pIn The bytes.
 * @param nInLength The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x002a05f8
 * @ghidraAddress PAL: 0x002aa568
 */
void Base64Encode(char *pszOut, int *pnOutLength, const unsigned char *pIn, int nInLength);

/**
 * Decode text that Base64Encode() wrote.
 *
 * The name is inferred.
 *
 * @param pOut Receives the bytes.
 * @param pnOutLength Receives the number of bytes.
 * @param pszIn The text.
 * @param nInLength The number of characters.
 * @ghidraAddress NTSC-U/C: 0x002a0700
 * @ghidraAddress PAL: 0x002aa670
 */
void Base64Decode(unsigned char *pOut, int *pnOutLength, const char *pszIn, int nInLength);
