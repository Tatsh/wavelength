#ifndef NETDEV_H
#define NETDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/** Memory services of the resident netdev library. */

/**
 * Allocate memory.
 *
 * @param mode Allocation mode, zero.
 * @param size Byte count.
 * @return The block, or null.
 */
void *NetdevAllocMemory(int mode, int size);

/**
 * Release memory from NetdevAllocMemory().
 *
 * @param mode Allocation mode given to NetdevAllocMemory().
 * @param block The block.
 * @return Zero, or a negative error code.
 */
int NetdevFreeMemory(int mode, void *block);

#ifdef __cplusplus
}
#endif

#endif
