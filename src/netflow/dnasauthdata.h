#pragma once

#ifdef VIDEO_STANDARD_PAL

/**
 * The pass phrase of the title that DNASTask passes to sceDNAS2InitNoHDD().
 *
 * The build copies the bytes from the original executable of the European release.
 *
 * @ghidraAddress PAL: 0x0042f950
 */
extern const unsigned char g_DNASPassPhrase[8];

/**
 * The size of g_DNASAuthData in bytes.
 *
 * @ghidraAddress PAL: 0x0041d190
 */
extern int g_nDNASAuthDataSize;

/**
 * The authentication data of the title that DNASTask passes to sceDNAS2InitNoHDD().
 *
 * The build copies the bytes from the original executable of the European release.
 *
 * @ghidraAddress PAL: 0x0041d198
 */
extern const unsigned char g_DNASAuthData[];

#endif
