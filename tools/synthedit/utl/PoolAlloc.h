#pragma once

#include "utl/Data.h"
#include "utl/PrnStream.h"

/**
 * Bytes taken through PoolChunkAlloc().
 *
 * @ghidraAddress 0x100c18cc
 */
extern int gPoolChunkBytes;

/**
 * Read the chunk sizes of the pools from the `pool` configuration.
 *
 * The first chunk is `big_hunk` bytes and every later one `small_hunk` bytes.
 *
 * @param config The `pool` configuration.
 * @ghidraAddress 0x100117e0
 */
void PoolAllocInit(DataArray *config);

/**
 * Carve memory for pool nodes from the current chunk, taking a new chunk when it is too small.
 *
 * The rest of a chunk too small for the request is abandoned.
 *
 * @param bytes The size in bytes, rounded down to whole words.
 * @return The memory.
 * @ghidraAddress 0x10011810
 */
void *PoolChunkAlloc(int bytes);

/**
 * Allocate an object from the pools.
 *
 * @param classSize The size of the object's class in bytes.
 * @param reqSize The size requested. It must equal the class size.
 * @param name The object's name for the memory tracker.
 * @param unused A value passed through and not read.
 * @return The object.
 * @ghidraAddress 0x10011c10
 */
void *PoolAlloc(int classSize, int reqSize, const char *name, int unused);

/**
 * Return an object to the pools.
 *
 * @param size The size it was allocated with.
 * @param mem The object.
 * @ghidraAddress 0x10011ca0
 */
void PoolFree(int size, void *mem);

/**
 * Print the use of the pools.
 *
 * @param stream The destination.
 * @ghidraAddress 0x10011d00
 */
void PoolReport(PrnStream &stream);

/**
 * Allocate from the pools, or from the current heap when the size exceeds 128 bytes.
 *
 * @param size The size in bytes.
 * @param name The allocation's name for the memory tracker.
 * @return The allocation, or null for a size of zero.
 * @ghidraAddress 0x10011d40
 */
void *_PoolAlloc(int size, const char *name);

/**
 * Free memory from _PoolAlloc().
 *
 * @param size The size it was allocated with.
 * @param mem The allocation, or null.
 * @ghidraAddress 0x10011d80
 */
void _PoolFree(int size, void *mem);
