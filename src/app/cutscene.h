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
 * @param pszPath The movie, with its device prefix.
 * @return Whether the movie file opened.
 * @ghidraAddress NTSC-U/C: 0x001aead0
 * @ghidraAddress PAL: 0x001b77b0
 */
bool PlayMovieFile(const char *pszPath);
