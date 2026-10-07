#ifndef SYSCLIB_H
#define SYSCLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Memory and string routines of the resident sysclib library. */

/**
 * Copy memory between regions that do not overlap.
 *
 * @param dest Destination.
 * @param src Source.
 * @param n Byte count.
 * @return @p dest.
 */
void *memcpy(void *dest, const void *src, size_t n);

/**
 * Fill memory with a byte.
 *
 * @param dest Destination.
 * @param c Byte value.
 * @param n Byte count.
 * @return @p dest.
 */
void *memset(void *dest, int c, size_t n);

/**
 * Compare two strings.
 *
 * @param left First string.
 * @param right Second string.
 * @return Zero when the strings are equal, otherwise the sign of the first difference.
 */
int strcmp(const char *left, const char *right);

#ifdef __cplusplus
}
#endif

#endif
