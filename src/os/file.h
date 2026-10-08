#pragma once

#include <vector>

#include "os/string.h"

/**
 * The word at `0x00100000`, the load address of the executable.
 *
 * Several routines take a development-tool path while the word is non-zero. A retail image has zero
 * there.
 */
constexpr int kImageFirstWord = 0;

/**
 * Open file of the host or the disc.
 *
 * The RTTI includes the class name. The class is polymorphic, each kind of storage implements it,
 * and the members after the destructor are pure in this class.
 */
class File {
public:
    /** Open modes of New(). */
    enum Mode {
        kModeRead = 1,  /*!< Read the file. */
        kModeWrite = 2, /*!< Write the file. */
    };

    /** The open flags New() takes to read even a disc file through AsyncFile. */
    static constexpr int kFlagNoArchive = 1;

    /**
     * Open a file.
     *
     * A file read from the disc comes from the archive through ArkFile unless nFlags is
     * kFlagNoArchive. Every other file is an AsyncFile. While the trace is on, the path of each
     * file opened to read is written to the trace log.
     *
     * @param pszPath The path.
     * @param nMode The open mode, one or more of Mode.
     * @param nFlags The open flags.
     * @return The file, or null when it did not open.
     * @ghidraAddress NTSC-U/C: 0x00289330
     * @ghidraAddress PAL: 0x00292b28
     */
    static File *New(const char *pszPath, int nMode, int nFlags);

    /**
     * Close the file.
     *
     * @ghidraAddress NTSC-U/C: 0x003a7e58
     * @ghidraAddress PAL: 0x00416b38
     */
    virtual ~File() {
    }

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
     * @param nOrigin The origin of the offset, 0 for the start, 1 for the position, and 2 for the
     *                end.
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

/** Status of a host file FileGetStat() reports. The name is inferred. */
struct FileStat {
    int mMode;                  /*!< File type and access bits. */
    int mSize;                  /*!< Size in bytes. */
    unsigned char mCreated[8];  /*!< Creation time. */
    unsigned char mAccessed[8]; /*!< Last access time. */
    unsigned char mModified[8]; /*!< Last modification time. */
};

/**
 * Files the gzip reader reads by descriptor, 128 entries.
 *
 * @ghidraAddress NTSC-U/C: 0x0047fdf0
 */
extern std::vector<File *> gFileTable;

/**
 * Format the device path of a game file.
 *
 * When the game runs from the disc, the path is `cdrom0:\` and the file upper-cased, with
 * backslashes for slashes and a `;1` version suffix. Otherwise it is `host0:` and the file.
 *
 * @param pszDest Receives the path.
 * @param nSize The size of pszDest, which is not checked.
 * @param pszFile The file, relative to the root of the game files, or null for an empty path.
 * @ghidraAddress NTSC-U/C: 0x002894e8
 * @ghidraAddress PAL: 0x00292ce0
 */
void FormatDevicePath(char *pszDest, int nSize, const char *pszFile);

/**
 * Build the device path of a game file.
 *
 * The name is inferred.
 *
 * @param path Receives the path FormatDevicePath() formats.
 * @param pszFile The file, relative to the root of the game files.
 * @ghidraAddress NTSC-U/C: 0x002894a8
 * @ghidraAddress PAL: 0x00292ca0
 */
void MakeDevicePath(String &path, const char *pszFile);

/**
 * Record the path the executable was started from, without its device and version.
 *
 * Also opens an AsyncFile with an empty path, which is retained and never read.
 *
 * @param pszPath The path the executable was started from.
 * @ghidraAddress NTSC-U/C: 0x00289128
 * @ghidraAddress PAL: 0x00292920
 */
void FileSetBootPath(const char *pszPath);

/**
 * Report the path FileSetBootPath() recorded.
 *
 * @return The path.
 * @ghidraAddress NTSC-U/C: 0x00289260
 * @ghidraAddress PAL: 0x00292a58
 */
const char *FileBootExecutable();

/**
 * Open the trace log that File::New() writes the path of each file read to, and turn the trace on.
 *
 * @param pszPath The path of the log, or null or empty for no log.
 * @ghidraAddress NTSC-U/C: 0x00289270
 * @ghidraAddress PAL: 0x00292a68
 */
void FileOpenTraceLog(const char *pszPath);

/**
 * Turn the trace of File::New() on or off.
 *
 * @param nEnabled Non-zero to trace.
 * @ghidraAddress NTSC-U/C: 0x002892b0
 * @ghidraAddress PAL: 0x00292aa8
 */
void FileSetTraceEnabled(int nEnabled);

/**
 * Report whether the trace of File::New() is on.
 *
 * @return Non-zero while the trace is on.
 * @ghidraAddress NTSC-U/C: 0x002892c0
 * @ghidraAddress PAL: 0x00292ab8
 */
int FileIsTraceEnabled();

/**
 * Read the file settings of the system configuration.
 *
 * @ghidraAddress NTSC-U/C: 0x002892d0
 * @ghidraAddress PAL: 0x00292ac8
 */
void FileInit();

/**
 * Close the trace log.
 *
 * @ghidraAddress NTSC-U/C: 0x002892f0
 * @ghidraAddress PAL: 0x00292ae8
 */
void FileTerminate();

/**
 * Note the first read of a file.
 *
 * The body does nothing.
 *
 * @param pszPath The device path of the file.
 * @ghidraAddress NTSC-U/C: 0x002894a0
 * @ghidraAddress PAL: 0x00292c98
 */
void FileNoteRead(const char *pszPath);

/**
 * Read bytes from a file of gFileTable.
 *
 * @param nDescriptor The index of the file.
 * @param pBuffer Receives the bytes.
 * @param nBytes The number of bytes.
 * @return The number of bytes read.
 * @ghidraAddress NTSC-U/C: 0x00289e40
 * @ghidraAddress PAL: 0x00293638
 */
int FileTableRead(int nDescriptor, void *pBuffer, int nBytes);

/**
 * Report the status of a host file.
 *
 * With a non-zero kImageFirstWord the status is cleared instead.
 *
 * @param pszPath The file, relative to the root of the game files.
 * @param pStat Receives the status.
 * @return The result of sceGetstat(), or 0.
 * @ghidraAddress NTSC-U/C: 0x00289eb8
 * @ghidraAddress PAL: 0x002936b0
 */
int FileGetStat(const char *pszPath, FileStat *pStat);

/**
 * Create a host directory.
 *
 * @param pszPath The directory, relative to the root of the game files.
 * @return The result of sceMkdir(), or -1 when the game runs from the disc.
 * @ghidraAddress NTSC-U/C: 0x00289fa8
 * @ghidraAddress PAL: 0x002937a0
 */
int FileMkDir(const char *pszPath);

/**
 * Report the letter the module loader puts in front of a module's source.
 *
 * @return `r` with a non-zero kImageFirstWord, otherwise `s`.
 * @ghidraAddress NTSC-U/C: 0x0028a178
 * @ghidraAddress PAL: 0x00293970
 */
char FileSourceLetter();
