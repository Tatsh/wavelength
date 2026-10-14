#ifndef LIBCCONSOLE_H
#define LIBCCONSOLE_H

#include <stddef.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Console device of the C library, moving standard input and output over the DECI2 TTY protocol.
 *
 * The routines have no published names. The C library's standard descriptors route to them.
 */

/**
 * Read a line from the console, for standard input only.
 *
 * @param nFile The descriptor.
 * @param pBuffer Receives the line.
 * @param nLength Size of pBuffer in bytes.
 * @return The number of bytes read, or -1 for any other descriptor.
 * @ghidraAddress NTSC-U/C: 0x00317568
 * @ghidraAddress PAL: 0x00383e28
 */
int reax(int nFile, void *pBuffer, int nLength);

/**
 * Write to the console, for standard output and standard error only.
 *
 * @param nFile The descriptor.
 * @param pBuffer The bytes to write.
 * @param nLength Size of pBuffer in bytes.
 * @return The number of bytes consumed, or -1 for any other descriptor.
 * @ghidraAddress NTSC-U/C: 0x003174e8
 * @ghidraAddress PAL: 0x00383da8
 */
int writx(int nFile, const void *pBuffer, int nLength);

/**
 * Close a console descriptor.
 *
 * @param nFile The descriptor.
 * @return Always -1.
 * @ghidraAddress NTSC-U/C: 0x003175e0
 * @ghidraAddress PAL: 0x00383ec8
 */
int LibcConsoleClose(int nFile);

/**
 * Seek a console descriptor.
 *
 * @param nFile The descriptor.
 * @param nOffset The offset.
 * @param nOrigin The origin of nOffset.
 * @return Always -1.
 * @ghidraAddress NTSC-U/C: 0x003175e8
 * @ghidraAddress PAL: 0x00383ed0
 */
int LibcConsoleLseek(int nFile, int nOffset, int nOrigin);

/**
 * Extend the heap by a number of bytes, with interrupts disabled.
 *
 * @param nIncrement Bytes to add.
 * @return The previous end of the heap, or -1 with errno set to ENOMEM when the heap would pass
 * EndOfHeap().
 * @ghidraAddress NTSC-U/C: 0x003175f0
 * @ghidraAddress PAL: 0x00383ed8
 */
void *sbrk(ptrdiff_t nIncrement);

/**
 * Report whether a descriptor is a terminal.
 *
 * @param nFile The descriptor.
 * @return Always 1.
 * @ghidraAddress NTSC-U/C: 0x003176a0
 * @ghidraAddress PAL: 0x00383f88
 */
int LibcConsoleIsatty(int nFile);

/**
 * Describe a descriptor as a character device.
 *
 * Only the mode and the modification time are written. The mode is S_IFCHR and the modification
 * time is zero.
 *
 * @param nFile The descriptor.
 * @param pStat Receives the description.
 * @return Always 0.
 * @ghidraAddress NTSC-U/C: 0x003176a8
 */
int LibcConsoleFstat(int nFile, struct stat *pStat);

/**
 * Mark the console closed. The next read or write opens the DECI2 TTY again.
 *
 * @ghidraAddress NTSC-U/C: 0x003173a0
 * @ghidraAddress PAL: 0x00383c60
 */
void LibcConsoleReset(void);

#ifdef __cplusplus
}
#endif

#endif
