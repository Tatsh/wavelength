#ifndef SYSCLIB_H
#define SYSCLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Memory and string routines of the resident sysclib library. */

/** Character class bits look_ctype_table() returns. */
enum CtypeClass {
    CTYPE_DIGIT = 0x04, /*!< A decimal digit. */
    CTYPE_SPACE = 0x08, /*!< White space. */
};

/**
 * Classify a character.
 *
 * @param c Character.
 * @return #CtypeClass bits.
 */
char look_ctype_table(char c);

/**
 * Convert the leading number of a string.
 *
 * @param text String.
 * @param end Receives the address after the number, or null.
 * @param base Number base.
 * @return The number.
 */
long strtol(const char *text, char **end, int base);

/**
 * Copy at most n characters of a string, padding the rest with terminators.
 *
 * @param dest Destination.
 * @param src Source.
 * @param n Byte count.
 * @return @p dest.
 */
char *strncpy(char *dest, const char *src, size_t n);

/**
 * Append a string.
 *
 * @param dest String to extend.
 * @param src String to append.
 * @return @p dest.
 */
char *strcat(char *dest, const char *src);

/**
 * Compare two regions byte by byte.
 *
 * @param left First region.
 * @param right Second region.
 * @param n Byte count.
 * @return Zero when the regions are equal, otherwise the sign of the first difference.
 */
int memcmp(const void *left, const void *right, size_t n);

/**
 * Copy memory, with the source first.
 *
 * @param src Source.
 * @param dest Destination.
 * @param n Byte count.
 */
void bcopy(const void *src, void *dest, size_t n);

/**
 * Clear memory.
 *
 * @param dest Destination.
 * @param n Byte count.
 */
void bzero(void *dest, size_t n);

/**
 * Write formatted text to a buffer.
 *
 * @param buffer Destination.
 * @param format Format string.
 * @return The number of characters written, not counting the terminator.
 */
int sprintf(char *buffer, const char *format, ...);

/**
 * Copy a string.
 *
 * @param dest Destination.
 * @param src Source.
 * @return @p dest.
 */
char *strcpy(char *dest, const char *src);

/**
 * Measure a string.
 *
 * @param text String.
 * @return The number of characters before the terminator.
 */
size_t strlen(const char *text);

/**
 * Compare at most n characters of two strings.
 *
 * @param left First string.
 * @param right Second string.
 * @param n Largest number of characters to compare.
 * @return Zero when the prefixes are equal, otherwise the sign of the first difference.
 */
int strncmp(const char *left, const char *right, size_t n);

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
