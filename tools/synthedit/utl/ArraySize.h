#pragma once

/**
 * The number of elements of an array, as an int.
 *
 * The argument must be an array, not a pointer.
 */
#define ARRAY_SIZE(array) (static_cast<int>(sizeof(array) / sizeof((array)[0])))
