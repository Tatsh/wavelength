#pragma once

/**
 * The line of the data file being parsed.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2370
 */
extern int g_nDataParseLineNumber;

/**
 * Supply the scanner with input from the stream being parsed.
 *
 * The name is inferred.
 *
 * @param pBuffer Receives the bytes.
 * @param nMaxSize The number of bytes to read.
 * @return nMaxSize, or 0 once the stream has failed before or after the read.
 * @ghidraAddress NTSC-U/C: 0x00297e38
 * @ghidraAddress PAL: 0x002a1a48
 */
int DataInput(char *pBuffer, int nMaxSize);
