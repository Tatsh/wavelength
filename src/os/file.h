#pragma once

#include "os/string.h"

/**
 * Build the device path of a game file.
 *
 * When the game runs from the disc, the path is `cdrom0:\` and the file upper-cased, with
 * backslashes for slashes and a `;1` version suffix. Otherwise it is `host0:` and the file. The
 * name is inferred.
 *
 * @param path Receives the path.
 * @param pszFile The file, relative to the root of the game files.
 * @ghidraAddress NTSC-U/C: 0x002894a8
 * @ghidraAddress PAL: 0x00292ca0
 */
void MakeDevicePath(String &path, const char *pszFile);

/**
 * Open file of the host or the disc.
 *
 * The class is polymorphic. Each kind of storage implements it, and the members after the
 * destructor are pure in this class.
 */
class File {
public:
    /**
     * Open a file of the kind its path and mode select.
     *
     * The name is inferred.
     *
     * @param pszPath The path.
     * @param nMode The open mode, 1 to read.
     * @param nFlags The open flags.
     * @return The file, or null when it did not open.
     * @ghidraAddress NTSC-U/C: 0x00289330
     * @ghidraAddress PAL: 0x00292b28
     */
    static File *New(const char *pszPath, int nMode, int nFlags);

    /** Close the file. */
    virtual ~File();

    /**
     * Read bytes and wait for them.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes.
     * @return The number of bytes read.
     */
    virtual int Read(void *pBuffer, int nBytes) = 0;

    /**
     * Start reading bytes. ReadDone() reports when they arrived.
     *
     * @param pBuffer Receives the bytes.
     * @param nBytes The number of bytes.
     * @return Whether the read started.
     */
    virtual bool ReadAsync(void *pBuffer, int nBytes) = 0;

    /**
     * Write bytes.
     *
     * @param pBuffer The bytes.
     * @param nBytes The number of bytes.
     * @return The number of bytes written.
     */
    virtual int Write(const void *pBuffer, int nBytes) = 0;

    /**
     * Move the position.
     *
     * @param nOffset The offset.
     * @param nOrigin The origin of the offset.
     * @return The new position.
     */
    virtual int Seek(int nOffset, int nOrigin) = 0;

    /**
     * Report the position.
     *
     * @return The position.
     */
    virtual int Tell() = 0;

    /** Write out buffered bytes. */
    virtual void Flush() = 0;

    /**
     * Report whether the position is at the end.
     *
     * @return Whether the end was arrived at.
     */
    virtual bool Eof() = 0;

    /**
     * Report whether an operation failed.
     *
     * @return Whether an operation failed.
     */
    virtual bool Fail() = 0;

    /**
     * Report the size.
     *
     * @return The size in bytes.
     */
    virtual int Size() = 0;

    /**
     * Report the size before compression.
     *
     * @return The size in bytes.
     */
    virtual int UncompressedSize() = 0;

    /**
     * Report whether the read ReadAsync() started is complete.
     *
     * @param pnBytes Receives the number of bytes read.
     * @return Whether the read is complete.
     */
    virtual bool ReadDone(int *pnBytes) = 0;
};
