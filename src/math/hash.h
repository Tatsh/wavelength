#pragma once

/**
 * Hash a string into the buckets of a table.
 *
 * Each character is added to 127 times the hash so far, reduced modulo the bucket count at every
 * step. The ark file table and the symbol table use it.
 *
 * @param pszText The string.
 * @param nBuckets The bucket count.
 * @return The bucket, or 0 for an empty string.
 * @ghidraAddress NTSC-U/C: 0x002903e8
 * @ghidraAddress PAL: 0x00299db0
 */
int HashString(const char *pszText, int nBuckets);
