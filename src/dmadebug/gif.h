#ifndef DMADEBUG_GIF_H
#define DMADEBUG_GIF_H

/**
 * GIF packet decoder of the DMAdebug packet checker.
 *
 * The decoder reads a GIF packet a quadword at a time, reporting each GIF tag and each GS register
 * write the packet makes. The routine names are inferred.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The quadword GIF_processQword() decodes.
 *
 * @ghidraAddress NTSC-U/C: 0x00519a18
 */
extern const unsigned int *g_pGifData;

/**
 * Report whether the current GIF tag has no loops left.
 *
 * @return 1 when the next quadword is a GIF tag, otherwise 0.
 * @ghidraAddress NTSC-U/C: 0x002a2738
 * @ghidraAddress PAL: 0x002ac6a8
 */
int GIF_isTagDone(void);

/**
 * Decode the quadword at g_pGifData as a GIF tag or as data of the current tag.
 *
 * @return The flags of the messages the quadword raised.
 * @ghidraAddress NTSC-U/C: 0x002a5950
 * @ghidraAddress PAL: 0x002af8c0
 */
int GIF_processQword(void);

#ifdef __cplusplus
}
#endif

#endif
