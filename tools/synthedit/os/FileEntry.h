#pragma once

/**
 * Entry of the archive's file table.
 *
 * The RTTI includes the class name. The object is 0x14 bytes.
 */
struct FileEntry {
    int mOffset;     /*!< Offset of the file from the start of the archive. */
    int mHashedName; /*!< Index of the file name in the archive's string hash table. */
    int mHashedPath; /*!< Index of the directory in the archive's string hash table. */
    int mSize;       /*!< Size of the file in the archive. */
    int mUCSize;     /*!< Size of the file before compression. */
};
