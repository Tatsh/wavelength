#pragma once

/**
 * Encode bytes as text, four characters for every three bytes.
 *
 * The first character is the digit of the alphabet `0-9A-Za-z. ` at the remainder of the length
 * divided by 3. The encoding of a final partial group reads the bytes past the end. The name is
 * inferred.
 *
 * @param pszText Receives the text, without a terminator.
 * @param pnLength Receives the number of characters.
 * @param pData The bytes.
 * @param nSize The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x002a05f8
 * @ghidraAddress PAL: 0x002aa568
 */
void EncodeBase64(char *pszText, int *pnLength, const char *pData, int nSize);

/**
 * Decode the text EncodeBase64() writes.
 *
 * The name is inferred.
 *
 * @param pData Receives the bytes.
 * @param pnSize Receives the number of bytes.
 * @param pszText The text.
 * @param nLength The number of characters.
 * @ghidraAddress NTSC-U/C: 0x002a0700
 * @ghidraAddress PAL: 0x002aa670
 */
void DecodeBase64(char *pData, int *pnSize, const char *pszText, int nLength);
