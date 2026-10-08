#pragma once

#include <list>
#include <vector>

#include "os/ArkFile.h"
#include "os/AsyncTask.h"
#include "os/Block.h"
#include "os/BlockRequest.h"

/**
 * Cache of archive blocks that satisfies the reads of ArkFile.
 *
 * The object is 0x18 bytes. One block loads at a time, into the least recently used buffer,
 * and the requests stay sorted by block.
 */
class BlockMgr {
public:
    /**
     * Take the block buffers.
     *
     * @ghidraAddress 0x1000f170
     */
    void Init();

    /**
     * Report the blocks a range of the archive covers.
     *
     * @param offset The offset of the range.
     * @param bytes The size of the range.
     * @param firstBlock Receives the first block.
     * @param numBlocks Receives the number of blocks.
     * @param blockSize Receives the size of a block.
     * @ghidraAddress 0x1000f280
     */
    void
    GetAssociatedBlocks(__int64 offset, int bytes, int &firstBlock, int &numBlocks, int &blockSize);

    /**
     * Drop every waiting read of a file, and every request left with no read unless its block is
     * loading.
     *
     * @param file The file.
     * @ghidraAddress 0x1000f300
     */
    void KillBlockRequests(ArkFile *file);

    /**
     * Report the data of a loaded block and mark the block as just used.
     *
     * @param blockNum The block.
     * @return The data, or null when the block is not loaded or still loading.
     * @ghidraAddress 0x1000f3b0
     */
    char *GetBlockData(int blockNum);

    /**
     * Queue a read on the request of its block, creating the request when needed.
     *
     * @param task The read.
     * @ghidraAddress 0x1000f3e0
     */
    void AddRequest(const AsyncTask &task);

    /**
     * Finish the loading block and satisfy its reads, then start loading the next requested
     * block.
     *
     * @ghidraAddress 0x1000f5c0
     */
    void Poll();

    /**
     * Find the buffer of a block.
     *
     * @param blockNum The block.
     * @return The buffer, or null.
     * @ghidraAddress 0x1000f720
     */
    Block *FindBlock(int blockNum);

    /**
     * Find the least recently used buffer.
     *
     * @return The buffer, or null when every buffer was used at the latest timestamp.
     * @ghidraAddress 0x1000f770
     */
    Block *GetOldestBlock();

    std::list<BlockRequest> mRequests; /*!< The requests, sorted by block. */
    std::vector<Block *> mBlocks;      /*!< The block buffers. */
    Block *mLoadingBlock;              /*!< The buffer being loaded, or null. */
    int mSectorsPerBlock;              /*!< Disc sectors in one block. */
};

/**
 * The block manager.
 *
 * @ghidraAddress 0x100c1110
 */
extern BlockMgr TheBlockMgr;
