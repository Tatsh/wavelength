#pragma once

#include <vector>

#include "os/ArkHash.h"
#include "os/FileEntry.h"
#include "utl/Str.h"

/**
 * Archive of the game files on the disc, `gen/main.ark`.
 *
 * The object is 0x48 bytes. The header stores the version, the file table, and the string hash
 * table of the file and directory names.
 */
class Archive {
public:
    /**
     * Open an archive and read its header.
     *
     * @param basename The path of the archive.
     * @ghidraAddress 0x1000d530
     */
    explicit Archive(const char *basename);

    /**
     * Find a file.
     *
     * @param name The file's path relative to the root.
     * @param offset Receives the offset of the file in the archive.
     * @param size Receives the size of the file.
     * @param ucSize Receives the size of the file before compression.
     * @return Whether the archive has the file. When the names hash but no entry matches, the
     * outputs are cleared.
     * @ghidraAddress 0x1000d5a0
     */
    bool GetFileInfo(const char *name, __int64 &offset, int &size, int &ucSize);

    /**
     * Read the header. Any version other than 2 is a failure.
     *
     * @ghidraAddress 0x1000d750
     */
    void Read();

    std::vector<FileEntry> mFileEntries; /*!< The file table. */
    ArkHash mHashTable;                  /*!< The file and directory names. */
    String mBasename;                    /*!< The path of the archive. */
    __int64 mArcBase;                    /*!< Offset added to every file offset. */
    int mReserved40;                     // +0x40, cleared by the constructor and not read.
};

/**
 * The archive, created by ArchiveInit() when the files come from the disc.
 *
 * @ghidraAddress 0x10040ccc
 */
extern Archive *gArchive;

/**
 * Clear the three words before #gArchive. No routine reads them.
 *
 * @ghidraAddress 0x1000d2c0
 */
void ArchivePreInit();

/**
 * Open the archive when the files come from the disc, and start the block manager.
 *
 * @ghidraAddress 0x1000d2e0
 */
void ArchiveInit();

/**
 * Find a file in the archive.
 *
 * @param name The file's path relative to the root.
 * @param offset Receives the offset of the file in the archive.
 * @param size Receives the size of the file.
 * @param ucSize Receives the size of the file before compression.
 * @return Whether the archive is open and has the file.
 * @ghidraAddress 0x1000d350
 */
bool ArchiveGetFileInfo(const char *name, __int64 &offset, int &size, int &ucSize);
