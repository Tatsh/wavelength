#pragma once

#include "memcard/memcardcbhandler.h"
#include "memcard/memcarddirentry.h"

/**
 * Advance the memory card command in flight.
 *
 * Without waiting, the routine queries the library whether the command finished. A finished command
 * reports its result to the handler that issued it, through the handler method for the command.
 * A command the routine does not know is reported as `Bad memcard func: %d`.
 *
 * @ghidraAddress NTSC-U/C: 0x0028b5c0
 * @ghidraAddress PAL: 0x00294dc0
 */
void MemcardPoll();

/**
 * Start reading the type, the free space, and the format of a card.
 *
 * MemcardCBHandler::OnGetInfo() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @param pnType Receives the card type.
 * @param pnFree Receives the free space in kilobytes.
 * @param pnFormat Receives non-zero for a formatted card.
 * @ghidraAddress NTSC-U/C: 0x0028b8c8
 * @ghidraAddress PAL: 0x002950c8
 */
void MemcardGetInfo(MemcardCBHandler *pHandler, int nPort, int *pnType, int *pnFree, int *pnFormat);

/**
 * Start opening a file.
 *
 * MemcardCBHandler::OnOpen() receives the file descriptor or the error.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @param pszName The file.
 * @param nMode The library's open mode bits.
 * @ghidraAddress NTSC-U/C: 0x0028b928
 * @ghidraAddress PAL: 0x00295128
 */
void MemcardOpen(MemcardCBHandler *pHandler, int nPort, const char *pszName, int nMode);

/**
 * Start closing a file.
 *
 * MemcardCBHandler::OnClose() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nFd The file descriptor.
 * @ghidraAddress NTSC-U/C: 0x0028b980
 * @ghidraAddress PAL: 0x00295180
 */
void MemcardClose(MemcardCBHandler *pHandler, int nFd);

/**
 * Start reading a file.
 *
 * MemcardCBHandler::OnRead() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nFd The file descriptor.
 * @param pBuffer Receives the bytes.
 * @param nSize The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x0028b9c0
 * @ghidraAddress PAL: 0x002951c0
 */
void MemcardRead(MemcardCBHandler *pHandler, int nFd, void *pBuffer, int nSize);

/**
 * Start writing a file.
 *
 * MemcardCBHandler::OnWrite() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nFd The file descriptor.
 * @param pData The bytes.
 * @param nSize The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x0028ba08
 * @ghidraAddress PAL: 0x00295208
 */
void MemcardWrite(MemcardCBHandler *pHandler, int nFd, const void *pData, int nSize);

/**
 * Start moving the position of a file.
 *
 * MemcardCBHandler::OnSeek() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nFd The file descriptor.
 * @param nOffset The offset.
 * @param nMode The origin of the offset.
 * @ghidraAddress NTSC-U/C: 0x0028ba50
 * @ghidraAddress PAL: 0x00295250
 */
void MemcardSeek(MemcardCBHandler *pHandler, int nFd, int nOffset, int nMode);

/**
 * Start creating a directory.
 *
 * MemcardCBHandler::OnMkdir() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @param pszName The directory.
 * @ghidraAddress NTSC-U/C: 0x0028ba98
 * @ghidraAddress PAL: 0x00295298
 */
void MemcardMkdir(MemcardCBHandler *pHandler, int nPort, const char *pszName);

/**
 * Start listing the entries that match a name.
 *
 * At most 20 entries are listed, whatever nMaxEntries requests. MemcardCBHandler::OnGetDir()
 * receives the count and the entries.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @param pszName The name. It may include wildcards.
 * @param nMaxEntries The number of entries requested.
 * @param nMode The library's listing mode.
 * @ghidraAddress NTSC-U/C: 0x0028baf0
 * @ghidraAddress PAL: 0x002952f0
 */
void MemcardGetDir(
    MemcardCBHandler *pHandler, int nPort, const char *pszName, int nMaxEntries, int nMode);

/**
 * Start deleting a file or an empty directory.
 *
 * MemcardCBHandler::OnDelete() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @param pszName The file or directory.
 * @ghidraAddress NTSC-U/C: 0x0028bb80
 * @ghidraAddress PAL: 0x00295380
 */
void MemcardDelete(MemcardCBHandler *pHandler, int nPort, const char *pszName);

/**
 * Start formatting a card.
 *
 * MemcardCBHandler::OnFormat() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @ghidraAddress NTSC-U/C: 0x0028bbd8
 * @ghidraAddress PAL: 0x002953d8
 */
void MemcardFormat(MemcardCBHandler *pHandler, int nPort);

/**
 * Start unformatting a card.
 *
 * MemcardCBHandler::OnUnformat() receives the result.
 *
 * @param pHandler The handler of the result.
 * @param nPort The memory card slot.
 * @ghidraAddress NTSC-U/C: 0x0028bc30
 * @ghidraAddress PAL: 0x00295430
 */
void MemcardUnformat(MemcardCBHandler *pHandler, int nPort);

/**
 * Report the localisation token of the name of a memory card slot.
 *
 * @param nPort The memory card slot.
 * @return The token, or an empty string for a slot without a name.
 * @ghidraAddress NTSC-U/C: 0x0028bfa8
 * @ghidraAddress PAL: 0x002958f8
 */
const char *MemcardGetSlotName(int nPort);
