#ifndef RTRINGBUFFER_H
#define RTRINGBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Byte ring buffer of the SCE-RT library. The routines report 0 for success, 1 when the buffer has
 * too few bytes or too little room, 2 for a null argument, 3 when the allocation failed, and 4 when
 * a compaction needs a scratch buffer the ring buffer does not have.
 */

/** A ring buffer. */
typedef struct {
    unsigned int uRead;       /*!< The offset of the first byte to read. */
    unsigned int uWrite;      /*!< The offset of the next byte to write. */
    unsigned int uSize;       /*!< The size of the storage. */
    unsigned int uUsed;       /*!< The bytes stored. */
    unsigned char *pbData;    /*!< The storage. */
    unsigned char *pbScratch; /*!< A buffer as large as the storage for compaction, or null. */
} RtRingBuffer;

/**
 * Allocate the storage of a ring buffer.
 *
 * @param pBuffer The ring buffer.
 * @param uSize The size of the storage.
 * @param pbScratch A buffer as large as the storage for compaction, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfb48
 * @ghidraAddress PAL: 0x002c9ab8
 */
int RtInitRingBuffer(RtRingBuffer *pBuffer, unsigned int uSize, unsigned char *pbScratch);

/**
 * Release the storage of a ring buffer.
 *
 * @param pBuffer The ring buffer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfbe8
 * @ghidraAddress PAL: 0x002c9b58
 */
int RtDestroyRingBuffer(RtRingBuffer *pBuffer);

/**
 * Empty a ring buffer.
 *
 * @param pBuffer The ring buffer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfc38
 * @ghidraAddress PAL: 0x002c9ba8
 */
int RtResetRingBuffer(RtRingBuffer *pBuffer);

/**
 * Drop bytes from the front of a ring buffer.
 *
 * @param pBuffer The ring buffer.
 * @param uSize The number of bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfc58
 * @ghidraAddress PAL: 0x002c9bc8
 */
int RtConsumeRingBuffer(RtRingBuffer *pBuffer, unsigned int uSize);

/**
 * Count bytes written straight to the storage as stored.
 *
 * @param pBuffer The ring buffer.
 * @param uSize The number of bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfd00
 * @ghidraAddress PAL: 0x002c9c70
 */
int RtCommitRingBuffer(RtRingBuffer *pBuffer, unsigned int uSize);

/**
 * Append bytes to a ring buffer.
 *
 * @param pBuffer The ring buffer.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfda8
 * @ghidraAddress PAL: 0x002c9d18
 */
int RtWriteRingBuffer(RtRingBuffer *pBuffer, const void *pData, unsigned int uSize);

/**
 * Copy bytes from the front of a ring buffer without removing them.
 *
 * @param pDest Receives the bytes.
 * @param pBuffer The ring buffer.
 * @param uSize The number of bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bfeb8
 * @ghidraAddress PAL: 0x002c9e28
 */
int RtPeekRingBuffer(void *pDest, const RtRingBuffer *pBuffer, unsigned int uSize);

/**
 * Move the bytes of a ring buffer to the start of its storage, with its scratch buffer.
 *
 * @param pBuffer The ring buffer.
 * @param bMove Non-zero to move the bytes, 0 only to rewind an empty buffer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bff98
 * @ghidraAddress PAL: 0x002c9f08
 */
int RtCompactRingBuffer(RtRingBuffer *pBuffer, unsigned char bMove);

/**
 * Move the bytes of a ring buffer to the start of its storage, through a scratch buffer.
 *
 * @param pBuffer The ring buffer.
 * @param bMove Non-zero to move the bytes, 0 only to rewind an empty buffer.
 * @param pbScratch The scratch buffer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bffe8
 * @ghidraAddress PAL: 0x002c9f58
 */
int RtCompactRingBufferWith(RtRingBuffer *pBuffer, unsigned char bMove, unsigned char *pbScratch);

/**
 * Report the bytes stored.
 *
 * @param pBuffer The ring buffer.
 * @param puSize Receives the bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c00b8
 * @ghidraAddress PAL: 0x002ca028
 */
int RtGetRingBufferUsed(const RtRingBuffer *pBuffer, unsigned int *puSize);

/**
 * Report the bytes that can be read in one piece.
 *
 * @param pBuffer The ring buffer.
 * @param puSize Receives the bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c00e0
 * @ghidraAddress PAL: 0x002ca050
 */
int RtGetRingBufferReadable(const RtRingBuffer *pBuffer, unsigned int *puSize);

/**
 * Report the room left.
 *
 * @param pBuffer The ring buffer.
 * @param puSize Receives the room in bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0138
 * @ghidraAddress PAL: 0x002ca0a8
 */
int RtGetRingBufferFree(const RtRingBuffer *pBuffer, unsigned int *puSize);

/**
 * Report the room that can be written in one piece.
 *
 * @param pBuffer The ring buffer.
 * @param puSize Receives the room in bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0168
 * @ghidraAddress PAL: 0x002ca0d8
 */
int RtGetRingBufferWritable(const RtRingBuffer *pBuffer, unsigned int *puSize);

/**
 * Report where the next byte to read is.
 *
 * @param pBuffer The ring buffer.
 * @param ppbRead Receives the address.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c01b8
 * @ghidraAddress PAL: 0x002ca128
 */
int RtGetRingBufferReadPointer(const RtRingBuffer *pBuffer, unsigned char **ppbRead);

/**
 * Report where the next byte to write is.
 *
 * @param pBuffer The ring buffer.
 * @param ppbWrite Receives the address.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0228
 * @ghidraAddress PAL: 0x002ca198
 */
int RtGetRingBufferWritePointer(const RtRingBuffer *pBuffer, unsigned char **ppbWrite);

/**
 * Report whether the bytes stored wrap past the end of the storage.
 *
 * @param pBuffer The ring buffer.
 * @param pbWrapped Receives non-zero when the bytes wrap.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0298
 * @ghidraAddress PAL: 0x002ca208
 */
int RtIsRingBufferWrapped(const RtRingBuffer *pBuffer, unsigned char *pbWrapped);

#ifdef __cplusplus
}
#endif

#endif
