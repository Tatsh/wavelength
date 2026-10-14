#ifndef DMADEBUG_VIF_H
#define DMADEBUG_VIF_H

/**
 * VIF packet decoder of the DMAdebug packet checker.
 *
 * The decoder pulls the data of a VIF0 or VIF1 transfer through DMA_step(), reporting each VIF
 * code, the VU memory writes of each UNPACK, and the GIF packets of DIRECT and DIRECTHL. The
 * routine names are inferred.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decode the transfer of the VIF channel in g_nDmaChannel to its end.
 *
 * The image does not call this routine.
 *
 * @return The flags of the messages the transfer raised.
 * @ghidraAddress NTSC-U/C: 0x002a7488
 * @ghidraAddress PAL: 0x002b13f8
 */
int VIF_disassemble(void);

#ifdef __cplusplus
}
#endif

#endif
