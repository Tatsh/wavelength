#ifndef EZMPEG_IPUDMA_H
#define EZMPEG_IPUDMA_H

#ifdef __cplusplus
extern "C" {
#endif

// The IPU input helpers of Sony's movie texture sample. The movie texture player feeds each frame
// of a movie to the IPU through a chain these helpers build. The unit name is inferred.

/**
 * Write a source chain DMA tag.
 *
 * @param tag The tag's first doubleword.
 * @param addr The address field.
 * @param id The tag ID.
 * @param qwc The quadword count.
 * @ghidraAddress NTSC-U/C: 0x0021b410
 * @ghidraAddress PAL: 0x00224228
 */
void scTag(unsigned long long *tag, int addr, unsigned int id, unsigned int qwc);

/**
 * Build the reference tags that send a buffer to the IPU in spans of at most 0xffff0 bytes.
 *
 * Every tag but the last is a REF, and the last is a REFE.
 *
 * @param tags The tags, one quadword each.
 * @param addr The buffer.
 * @param size The buffer's size in bytes.
 * @ghidraAddress NTSC-U/C: 0x0021b460
 * @ghidraAddress PAL: 0x00224278
 */
void setToIPUtags(unsigned long long *tags, unsigned int addr, int size);

/**
 * Start the channel to the IPU in chain mode on a tag chain.
 *
 * @param tags The first tag.
 * @ghidraAddress NTSC-U/C: 0x0021b520
 * @ghidraAddress PAL: 0x00224338
 */
void setDMAtoIPU(unsigned int tags);

/**
 * Read the next byte of the bitstream and step the IPU past it.
 *
 * @return The byte.
 * @ghidraAddress NTSC-U/C: 0x0021b560
 * @ghidraAddress PAL: 0x00224378
 */
unsigned int getFlags(void);

#ifdef __cplusplus
}
#endif

#endif
