#pragma once

#include "utl/Str.h"

/**
 * An open file, read from the archive, the disc, or the host.
 *
 * The RTTI records the class. It has no members, and its subclasses provide every operation.
 */
class File {
public:
    /**
     * Close the file.
     *
     * @ghidraAddress 0x1000dec0
     */
    virtual ~File();

    /**
     * Read bytes and wait for them.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @return The number of bytes read.
     */
    virtual int Read(void *data, int bytes) = 0;

    /**
     * Start a read that ReadDone() finishes.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @return Whether the read started.
     */
    virtual bool ReadAsync(void *data, int bytes) = 0;

    /**
     * Write bytes.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     * @return The number of bytes written.
     */
    virtual int Write(const void *data, int bytes) = 0;

    /**
     * Move the position.
     *
     * @param offset The offset in bytes.
     * @param origin 0 from the start, 1 from the position, 2 from the end.
     * @return The new position, or a negative value on failure.
     */
    virtual int Seek(int offset, int origin) = 0;

    /** @return The position in bytes. */
    virtual int Tell() = 0;

    /** Write out buffered data. */
    virtual void Flush() = 0;

    /** @return Whether the position is at the end. */
    virtual bool Eof() = 0;

    /** @return Whether an operation failed. */
    virtual bool Fail() = 0;

    /** @return The size of the file as stored, in bytes. */
    virtual int Size() = 0;

    /** @return The size of the file after decompression, in bytes. */
    virtual int UncompressedSize() = 0;

    /**
     * Report whether the read ReadAsync() started has finished.
     *
     * @param bytes Receives the number of bytes read.
     * @return Whether the read has finished.
     */
    virtual bool ReadDone(int *bytes) = 0;
};

/** Bits of the open mode of NewFile(). The low bits pass through to `_open`. */
enum FileOpenMode {
    FILE_OPEN_WRITE = 1,        /*!< Open for writing. */
    FILE_OPEN_READ = 2,         /*!< Open for reading. */
    FILE_OPEN_CREATE = 0x100,   /*!< Create the file when it does not exist. */
    FILE_OPEN_TRUNCATE = 0x200, /*!< Discard the contents of an existing file. */
};

/**
 * Open a file from the archive or the host.
 *
 * A read from the disc comes from the archive unless the flags are 1. When the file log is open,
 * each read appends the file's path relative to the root to the log.
 *
 * @param name The path.
 * @param mode The open mode, a combination of FileOpenMode bits.
 * @param flags 1 to bypass the archive.
 * @return The file, or null when it did not open.
 * @ghidraAddress 0x1000a9a0
 */
File *NewFile(const char *name, int mode, int flags);

/**
 * Open the file log, which lists every file the program reads.
 *
 * @param file The path of the log, or null or empty for no log.
 * @ghidraAddress 0x1000a940
 */
void FileLogInit(const char *file);

/**
 * Read the asynchronous file configuration.
 *
 * @ghidraAddress 0x1000a970
 */
void FileInit();

/**
 * Close the file log.
 *
 * @ghidraAddress 0x1000a980
 */
void FileTerminate();

/**
 * Store a path as the host opens it. On Windows the path is unchanged.
 *
 * @param out Receives the path.
 * @param path The path.
 * @ghidraAddress 0x1000aad0
 */
void FileMakeLocalPath(String &out, const char *path);

/**
 * Copy a path, or store an empty string for a null path.
 *
 * @param out Receives the path.
 * @param size The size of #out. The copy does not limit itself to it.
 * @param path The path, or null.
 * @ghidraAddress 0x1000ab10
 */
void FileCopyPath(char *out, int size, const char *path);

/**
 * Report the working directory at the first call, normalised.
 *
 * @return The directory, in a static buffer.
 * @ghidraAddress 0x1000ab50
 */
const char *FileRoot();

/**
 * Lower-case a path, turn backslashes into slashes, and resolve `.` and `..` components.
 *
 * Every component that starts with a dot and is not `..` is dropped. A path that ends in `..`
 * normalises to the empty string.
 *
 * @param path The path.
 * @return The normalised path, in a static buffer.
 * @ghidraAddress 0x1000abb0
 */
const char *FileNormalizePath(const char *path);

/**
 * Express a path relative to a directory, both normalised.
 *
 * @param path The path.
 * @param root The directory.
 * @return The relative path, in a static buffer.
 * @ghidraAddress 0x1000ad50
 */
const char *FileRelativePath(const char *path, const char *root);

/**
 * Report whether a path is absolute.
 *
 * @param path The path, or null.
 * @return Whether the path is null, empty, starts with a slash or a backslash, or has a drive.
 * @ghidraAddress 0x1000b1e0
 */
bool FileIsAbsolute(const char *path);

/**
 * Report the directory of a path.
 *
 * @param path The path, or null.
 * @return The directory, or `.` when the path has none, in a static buffer.
 * @ghidraAddress 0x1000b210
 */
const char *FileGetPath(const char *path);

/**
 * Report the extension of a path.
 *
 * @param path The path.
 * @return The text after the last dot, or the empty string at the end of the path.
 * @ghidraAddress 0x1000b290
 */
const char *FileGetExt(const char *path);

/**
 * Report the file name of a path without its directory or extension.
 *
 * @param path The path.
 * @return The name, in a static buffer.
 * @ghidraAddress 0x1000b2c0
 */
const char *FileGetBase(const char *path);

/**
 * Express a path relative to the root, as the archive indexes it.
 *
 * @param path The path, absolute or relative to the root.
 * @return The relative path, in a static buffer.
 * @ghidraAddress 0x1000e520
 */
const char *FileLocalize(const char *path);
