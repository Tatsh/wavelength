#pragma once

/**
 * Play a PSS movie to its end, or until the player skips it.
 *
 * The routine allocates every decoder buffer, binds the IOP heap, plays the stream, and frees the
 * buffers again. A path without a device prefix is read through the host link. A path on the
 * `cdrom0` device is upper-cased, takes backslashes for slashes, and gains a `;1` version suffix
 * when it does not have one.
 *
 * Once the movie has played for a little over five seconds, pressing Cross or Start stops it.
 *
 * The name is inferred. Metagame plays the intro movie through it.
 *
 * The PAL build shows the movie on a 512-line PAL display, or on a 448-line NTSC display when
 * bNtsc is set, and can initialise the IOP sound library before the audio starts.
 *
 * @param pszPath The movie, with its device prefix.
 * @param bNtsc Nonzero for NTSC output, in the PAL build only.
 * @param bInitSound Nonzero to initialise the IOP sound library first, in the PAL build only.
 * @return Whether the movie file opened.
 * @ghidraAddress NTSC-U/C: 0x001aead0
 * @ghidraAddress PAL: 0x001b77b0
 */
#ifdef VIDEO_STANDARD_PAL
bool PlayMovieFile(const char *pszPath, int bNtsc, int bInitSound);
#else
bool PlayMovieFile(const char *pszPath);
#endif
