#pragma once

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

/**
 * List the entries of the memory card of a device that match a name, waiting for the result.
 *
 * The name is inferred. At most 20 entries are listed.
 *
 * @param nDevice The device.
 * @param pszName The path, optionally ending in a wildcard.
 * @param nMaxEntries The most entries to list.
 * @param nMode Zero to start a listing, nonzero to continue it.
 * @param pnCount Receives the number of entries listed.
 * @return The number of entries listed, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x0028be90
 * @ghidraAddress PAL: 0x00295690
 */
int MemcardGetDirAndWait(
    int nDevice, const char *pszName, int nMaxEntries, unsigned int nMode, int *pnCount);
