#pragma once

/**
 * Hash a string into a table.
 *
 * The source file of the routine is not recorded. The name is inferred.
 *
 * @param str The string.
 * @param tableSize The number of slots.
 * @return The slot, from zero to one less than the number of slots.
 * @ghidraAddress 0x1001bd90
 */
int HashString(const char *str, int tableSize);
