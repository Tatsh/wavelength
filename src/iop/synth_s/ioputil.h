#pragma once

/**
 * Allocate IOP memory from the lowest free area.
 *
 * @param size Byte count.
 * @return The block, or null when none fits.
 * @ghidraAddress NTSC-U/C: 0x00005fe0
 * @ghidraAddress PAL: 0x00005fe0
 */
void *IopAlloc(int size);

/**
 * Release a block IopAlloc() returned.
 *
 * @param block The block.
 * @ghidraAddress NTSC-U/C: 0x00006008
 * @ghidraAddress PAL: 0x00006008
 */
void IopFree(void *block);

/**
 * Create a dormant C thread with the module's stack size.
 *
 * @param entry Entry point.
 * @param priority Starting priority, where a smaller value runs first.
 * @return The thread identifier, or a negative error code.
 * @ghidraAddress NTSC-U/C: 0x00005e18
 * @ghidraAddress PAL: 0x00005e18
 */
int CreateSynthThread(void (*entry)(), int priority);

/**
 * Write an SPU2 parameter register.
 *
 * @param entry Entry value, a register combined with a voice and a core.
 * @param value Value to write.
 * @ghidraAddress NTSC-U/C: 0x00006030
 * @ghidraAddress PAL: 0x00006030
 */
void SpuSetParam(unsigned short entry, unsigned short value);
