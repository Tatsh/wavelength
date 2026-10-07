#pragma once

/**
 * Report the directory part of a path.
 *
 * The result is the path up to its last forward or backward slash, or `.` when the path has no
 * slash or is null.
 *
 * @param pszPath The path, or null.
 * @return A shared buffer the next call replaces.
 * @ghidraAddress NTSC-U/C: 0x00289cd8
 * @ghidraAddress PAL: 0x002934d0
 */
const char *FileGetPath(const char *pszPath);

/**
 * Report the directory the game's files are read from, `.`.
 *
 * The name is inferred.
 *
 * @return The directory.
 * @ghidraAddress NTSC-U/C: 0x002895b8
 * @ghidraAddress PAL: 0x00292db0
 */
const char *FileRoot();
