#pragma once

#include <vector>

#include "os/arkhash.h"
#include "os/string.h"

/**
 * Record of one file of an archive.
 *
 * The RTTI includes the class name.
 */
struct FileEntry {
    int mOffset;           /*!< The first byte, from the start of the archive. */
    int mHashedName;       /*!< The ArkHash slot of the file's name. */
    int mHashedPath;       /*!< The ArkHash slot of the file's directory, or -1. */
    int mSize;             /*!< The size in bytes. */
    int mUncompressedSize; /*!< The size before compression. */
};

/**
 * The archive on the disc that the game files are read from.
 *
 * The class is not polymorphic and emits no RTTI, and the name is inferred.
 */
class Archive {
public:
    /**
     * Find an archive on the disc and read its header from the host or disc file system.
     *
     * @param pszBasename The archive, relative to the root of the game files.
     * @ghidraAddress NTSC-U/C: 0x0028e158
     * @ghidraAddress PAL: 0x00297b20
     */
    explicit Archive(const char *pszBasename);

    /**
     * Open the main archive when the game runs from the disc, and build the disc cache.
     *
     * @ghidraAddress NTSC-U/C: 0x0028deb0
     * @ghidraAddress PAL: 0x00297890
     */
    static void Init();

    /**
     * Find a file in the main archive.
     *
     * @param pszFile The file, as ArkFile::ArchivePath() reports it.
     * @param pnOffset Receives the byte of the disc the file starts at.
     * @param pnSize Receives the size.
     * @param pnUncompressedSize Receives the size before compression.
     * @return Whether the file was found, false without a main archive.
     * @ghidraAddress NTSC-U/C: 0x0028df10
     * @ghidraAddress PAL: 0x002978f0
     */
    static bool
    Locate(const char *pszFile, long long *pnOffset, int *pnSize, int *pnUncompressedSize);

    /**
     * Find a file in the archive.
     *
     * @param pszFile The file, as ArkFile::ArchivePath() reports it.
     * @param pnOffset Receives the byte of the disc the file starts at, or 0.
     * @param pnSize Receives the size, or 0.
     * @param pnUncompressedSize Receives the size before compression, or 0.
     * @return Whether the file was found.
     * @ghidraAddress NTSC-U/C: 0x0028e2a0
     * @ghidraAddress PAL: 0x00297c68
     */
    bool
    GetFileInfo(const char *pszFile, long long *pnOffset, int *pnSize, int *pnUncompressedSize);

    /**
     * Read the file records and the name table of the archive.
     *
     * @ghidraAddress NTSC-U/C: 0x0028e488
     * @ghidraAddress PAL: 0x00297e50
     */
    void ReadHeader();

    std::vector<FileEntry> mFileEntries; /*!< The file records. */
    ArkHash mHashTable;                  /*!< The names of the files and directories. */
    String mBasename;        /*!< The archive, relative to the root of the game files. */
    long long mArkfileStart; /*!< The byte of the disc the archive starts at. */
    int mReserved40;         // +0x40, cleared by the constructor and otherwise unused
};

/**
 * The main archive, or null when the game does not run from the disc.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2274
 */
extern Archive *TheArchive;
