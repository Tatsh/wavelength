#pragma once

#ifdef VIDEO_STANDARD_PAL

/**
 * Report the bytes a text of the console's character set takes in UTF-8, the terminator included.
 *
 * A character above 127 takes two bytes and any other character one. The name is inferred.
 *
 * @param pszText The text.
 * @return The size in bytes.
 * @ghidraAddress PAL: 0x002aa2b0
 */
int Utf8Size(const char *pszText);

#endif
