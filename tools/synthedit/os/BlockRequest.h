#pragma once

#include <list>

#include "os/AsyncTask.h"

/**
 * Reads waiting for one block of the archive.
 *
 * The RTTI includes the class name. The object is 8 bytes.
 */
class BlockRequest {
public:
    /**
     * Start the list of reads of a block with one read.
     *
     * @param blockNum The block.
     * @param task The read.
     * @ghidraAddress 0x1000f0c0
     */
    BlockRequest(int blockNum, const AsyncTask &task);

    /**
     * Release the list of reads.
     *
     * @ghidraAddress 0x1000f550
     */
    ~BlockRequest();

    int mBlockNum;               /*!< The block. */
    std::list<AsyncTask> mTasks; /*!< The reads waiting for the block. */
};
