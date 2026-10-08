#pragma once

#include "os/ArkFile.h"

/**
 * Part of an archive read that one block satisfies.
 *
 * The RTTI includes the class name. The object is 0x14 bytes.
 */
class AsyncTask {
public:
    /**
     * Describe the part of a read that one block satisfies.
     *
     * @param file The file that reads.
     * @param buffer Receives the bytes.
     * @param blockNum The block.
     * @param start The offset of the first byte in the block.
     * @param end The offset one past the last byte in the block.
     * @ghidraAddress 0x10010000
     */
    AsyncTask(ArkFile *file, char *buffer, int blockNum, int start, int end);

    /**
     * Copy the bytes when the block is loaded, and tell the file.
     *
     * @return Whether the block was loaded.
     * @ghidraAddress 0x10010030
     */
    bool FillData();

    int mBlockNum;  /*!< The block. */
    int mStart;     /*!< The offset of the first byte in the block. */
    int mEnd;       /*!< The offset one past the last byte in the block. */
    char *mBuffer;  /*!< Receives the bytes. */
    ArkFile *mFile; /*!< The file that reads. */
};
