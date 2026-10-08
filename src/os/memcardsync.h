#pragma once

#include "memcard/memcarddirentry.h"

/**
 * Report the state of the memory card of a device, waiting for the result.
 *
 * The name is inferred. The device selects a port and a slot from a table.
 *
 * @param nDevice The device.
 * @param pnType Receives the card type, or null.
 * @param pnFree Receives the free clusters, or null.
 * @param pnFormat Receives whether the card is formatted, or null.
 * @return The result of the request.
 * @ghidraAddress NTSC-U/C: 0x0028bcd8
 * @ghidraAddress PAL: 0x002954d8
 */
int MemcardGetInfoAndWait(int nDevice, int *pnType, int *pnFree, int *pnFormat);

/** Flags of MemcardOpenAndWait(). */
enum MemcardOpenFlags {
    kMemcardOpenRead = 0x0001,   /*!< Open for reading. */
    kMemcardOpenWrite = 0x0002,  /*!< Open for writing. */
    kMemcardOpenCreate = 0x0200, /*!< Create the file when it does not exist. */
};

/**
 * Open a file of the memory card of a device, waiting for the result.
 *
 * The name is inferred.
 *
 * @param nDevice The device.
 * @param pszPath The path of the file.
 * @param nFlags A set of MemcardOpenFlags.
 * @return The descriptor of the file, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x0028bd50
 * @ghidraAddress PAL: 0x00295550
 */
int MemcardOpenAndWait(int nDevice, const char *pszPath, int nFlags);

/**
 * Close a file of a memory card, waiting for the result.
 *
 * The name is inferred.
 *
 * @param nFile The descriptor of the file.
 * @return The result of the request.
 * @ghidraAddress NTSC-U/C: 0x0028bdb8
 * @ghidraAddress PAL: 0x002955b8
 */
int MemcardCloseAndWait(int nFile);

/**
 * Write to a file of a memory card, waiting for the result.
 *
 * The name is inferred.
 *
 * @param nFile The descriptor of the file.
 * @param pData The bytes.
 * @param nBytes The number of bytes.
 * @return The bytes written, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x0028be18
 * @ghidraAddress PAL: 0x00295618
 */
int MemcardWriteAndWait(int nFile, const void *pData, int nBytes);

/**
 * List the entries of the memory card of a device that match a name, waiting for the result.
 *
 * The name is inferred. At most 20 entries are listed. The listing callback records the entries
 * and not the count, and the call therefore always returns -1.
 *
 * @param nDevice The device.
 * @param pszName The path, optionally ending in a wildcard.
 * @param nMaxEntries The most entries to list.
 * @param nMode Zero to start a listing, nonzero to continue it.
 * @param ppEntries Receives the entries.
 * @return -1.
 * @ghidraAddress NTSC-U/C: 0x0028be90
 * @ghidraAddress PAL: 0x00295690
 */
int MemcardGetDirAndWait(
    int nDevice, const char *pszName, int nMaxEntries, int nMode, MemcardDirEntry **ppEntries);
