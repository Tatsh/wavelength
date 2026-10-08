#pragma once

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

/**
 * Normalise a path.
 *
 * The path is lower-cased, its backslashes become slashes, and its `.` components and every `..`
 * component with a preceding component to cancel are removed. A path whose last such component
 * cancels a preceding one normalises to the empty string. A leading slash is preserved.
 *
 * @param pszPath The path.
 * @return A shared buffer the next call replaces.
 * @ghidraAddress NTSC-U/C: 0x002895c8
 * @ghidraAddress PAL: 0x00292dc0
 */
const char *FileNormalizePath(const char *pszPath);

/**
 * Express a path relative to a base directory.
 *
 * Both paths are split at their slashes. The components they share at the start are dropped, a
 * `..` replaces each remaining component of the base, and the remaining components of the path
 * follow.
 *
 * @param pszPath The path.
 * @param pszBase The base directory.
 * @return A shared buffer the next call replaces, empty for an empty path.
 * @ghidraAddress NTSC-U/C: 0x002897c0
 * @ghidraAddress PAL: 0x00292fb8
 */
const char *FileRelativePath(const char *pszPath, const char *pszBase);

/**
 * Report whether a path is absolute.
 *
 * A path is absolute when it starts with a slash or a backslash or has a colon as its second
 * character. A null or empty path is treated as absolute.
 *
 * @param pszPath The path, or null.
 * @return Whether the path is absolute.
 * @ghidraAddress NTSC-U/C: 0x00289c88
 * @ghidraAddress PAL: 0x00293480
 */
bool FileIsAbsolute(const char *pszPath);

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
 * Report the extension of a path.
 *
 * The result is the text after the last full stop, or the empty text at the end of a path without
 * a full stop.
 *
 * @param pszPath The path.
 * @return A pointer into the path.
 * @ghidraAddress NTSC-U/C: 0x00289d60
 * @ghidraAddress PAL: 0x00293558
 */
const char *FileGetExt(const char *pszPath);

/**
 * Report the file name of a path without its directory and its extension.
 *
 * @param pszPath The path.
 * @return A shared buffer the next call replaces.
 * @ghidraAddress NTSC-U/C: 0x00289da0
 * @ghidraAddress PAL: 0x00293598
 */
const char *FileGetBaseName(const char *pszPath);
