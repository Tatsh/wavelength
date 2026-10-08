#pragma once

/**
 * Read sectors of the archive, standing in for the disc.
 *
 * The read completes before the routine returns.
 *
 * @param sector The first sector.
 * @param numSectors The number of sectors.
 * @param buffer Receives the sectors.
 * @return Always zero.
 * @ghidraAddress 0x10010080
 */
int CDReadSectors(int sector, int numSectors, void *buffer);

/**
 * Report whether the last read finished. Reads finish at once, so the result is always true.
 *
 * The linker merged the routine with AsyncFile::_ReadDone().
 *
 * @return Always true.
 * @ghidraAddress 0x1000ffe0
 */
bool CDReadDone();

/**
 * Report the error of the last read. Reads do not fail, so the result is always zero.
 *
 * The linker merged the routine with every other routine that only returns zero.
 *
 * @return Always zero.
 * @ghidraAddress 0x1000eed0
 */
int CDGetError();
