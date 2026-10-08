#pragma once

#include <list>
#include <vector>

#include "os/arkfile.h"

/**
 * One block buffer of the disc cache.
 *
 * The class is not polymorphic and emits no RTTI, and the name is inferred. Each block takes the
 * next of eight static 64 KB buffers.
 */
class Block {
public:
    /**
     * Take the next buffer and mark the block used now.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f7f0
     * @ghidraAddress PAL: 0x002991b8
     */
    Block();

    /**
     * Report the index of the next buffer a block takes, and advance it.
     *
     * @return The index.
     * @ghidraAddress NTSC-U/C: 0x0028f7d8
     * @ghidraAddress PAL: 0x002991a0
     */
    static int NextBuffer();

    /**
     * Report the latest use mark Touch() handed out.
     *
     * @return The mark.
     * @ghidraAddress NTSC-U/C: 0x0028f840
     * @ghidraAddress PAL: 0x00299208
     */
    static int CurrentTime();

    /**
     * Mark the block used now.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f850
     * @ghidraAddress PAL: 0x00299218
     */
    void Touch();

    char *mBuffer;  /*!< The block's data. */
    int mBlockNum;  /*!< The block of the disc the buffer holds, or -1. */
    int mTimestamp; /*!< The use mark of the last Touch(). */
};

/**
 * One part of an ArkFile read, which a single block of the disc satisfies.
 *
 * The RTTI of the list node includes the class name.
 */
struct AsyncTask {
    /**
     * Describe the part of a read.
     *
     * @param pFile The file reading.
     * @param pBuffer Receives the bytes.
     * @param nBlock The block of the disc.
     * @param nStart The first byte in the block.
     * @param nEnd The byte after the last in the block.
     * @ghidraAddress NTSC-U/C: 0x0028f748
     * @ghidraAddress PAL: 0x00299110
     */
    AsyncTask(ArkFile *pFile, char *pBuffer, int nBlock, int nStart, int nEnd);

    /**
     * Copy the bytes from the cache when their block is loaded, and report them to the file.
     *
     * @return Whether the block was loaded.
     * @ghidraAddress NTSC-U/C: 0x0028f768
     * @ghidraAddress PAL: 0x00299130
     */
    bool TryComplete();

    int mBlock;     /*!< The block of the disc. */
    int mStart;     /*!< The first byte in the block. */
    int mEnd;       /*!< The byte after the last in the block. */
    char *mBuffer;  /*!< Receives the bytes. */
    ArkFile *mFile; /*!< The file reading. */
};

/**
 * The tasks waiting for one block of the disc.
 *
 * The RTTI of the list node includes the class name.
 */
struct BlockRequest {
    /**
     * Start a request with its first task.
     *
     * @param nBlock The block of the disc.
     * @param task The first task.
     * @ghidraAddress NTSC-U/C: 0x0028f868
     * @ghidraAddress PAL: 0x00299230
     */
    BlockRequest(int nBlock, const AsyncTask &task);

    int mBlockNum;               /*!< The block of the disc. */
    std::list<AsyncTask> mTasks; /*!< The tasks waiting for the block. */
};

/**
 * Cache of disc blocks that ArkFile reads go through.
 *
 * The class is not polymorphic and emits no RTTI, and the name is inferred. One block loads at a
 * time. Requests are kept sorted by block, and the oldest unused block takes the next load.
 */
class BlockMgr {
public:
    /**
     * Build the blocks of the cache.
     *
     * @ghidraAddress NTSC-U/C: 0x0028f978
     * @ghidraAddress PAL: 0x00299340
     */
    void Init();

    /**
     * Report the blocks a run of bytes of the disc covers.
     *
     * @param nOffset The first byte.
     * @param nBytes The number of bytes.
     * @param pnFirst Receives the first block.
     * @param pnCount Receives the number of blocks, at least 1.
     * @param pnBlockSize Receives the size of a block in bytes.
     * @ghidraAddress NTSC-U/C: 0x0028fa80
     * @ghidraAddress PAL: 0x00299448
     */
    void GetBlockSpan(long long nOffset, int nBytes, int *pnFirst, int *pnCount, int *pnBlockSize);

    /**
     * Drop every task of a file, and every request left with no task unless its block is loading.
     *
     * @param pFile The file.
     * @ghidraAddress NTSC-U/C: 0x0028fb58
     * @ghidraAddress PAL: 0x00299520
     */
    void KillBlockRequests(ArkFile *pFile);

    /**
     * Report the byte of the disc a sector starts at.
     *
     * @param nSector The sector.
     * @return The byte.
     * @ghidraAddress NTSC-U/C: 0x0028fcf8
     * @ghidraAddress PAL: 0x002996c0
     */
    int SectorToByte(int nSector);

    /**
     * Report the data of a loaded block and mark the block used.
     *
     * @param nBlock The block of the disc.
     * @return The data, or null when the block is not loaded or still loading.
     * @ghidraAddress NTSC-U/C: 0x0028fd00
     * @ghidraAddress PAL: 0x002996c8
     */
    char *GetBlockData(int nBlock);

    /**
     * Queue a task, joining the request for its block or inserting a new one in block order.
     *
     * @param task The task.
     * @ghidraAddress NTSC-U/C: 0x0028fd50
     * @ghidraAddress PAL: 0x00299718
     */
    void AddRequest(const AsyncTask &task);

    /**
     * Finish the block that loaded, completing its tasks, and start loading the next request.
     *
     * A read that failed is started again.
     *
     * @ghidraAddress NTSC-U/C: 0x0028ff88
     * @ghidraAddress PAL: 0x00299950
     */
    void Poll();

    /**
     * Find the block holding a block of the disc.
     *
     * @param nBlock The block of the disc.
     * @return The block, or null.
     * @ghidraAddress NTSC-U/C: 0x002901a8
     * @ghidraAddress PAL: 0x00299b70
     */
    Block *FindBlock(int nBlock);

    /**
     * Find the block used longest ago.
     *
     * @return The block, or null when every block was used at the latest mark.
     * @ghidraAddress NTSC-U/C: 0x002901f8
     * @ghidraAddress PAL: 0x00299bc0
     */
    Block *FindOldestBlock();

    /**
     * Report whether the read of the drive finished.
     *
     * @return Whether the read finished.
     * @ghidraAddress NTSC-U/C: 0x00290378
     * @ghidraAddress PAL: 0x00299d40
     */
    static bool ReadDone();

    /**
     * Report the error of the last read of the drive.
     *
     * @return The sceCdGetError() code.
     * @ghidraAddress NTSC-U/C: 0x00290398
     * @ghidraAddress PAL: 0x00299d60
     */
    static int ReadError();

    /**
     * Start reading sectors of the disc.
     *
     * @param nSector The first sector.
     * @param nCount The number of sectors.
     * @param pBuffer Receives the sectors.
     * @return Whether the read started.
     * @ghidraAddress NTSC-U/C: 0x002903b8
     * @ghidraAddress PAL: 0x00299d80
     */
    static bool Read(int nSector, int nCount, void *pBuffer);

    std::list<BlockRequest> mRequests; /*!< The waiting requests in block order. */
    std::vector<Block *> mBlocks;      /*!< The blocks of the cache. */
    Block *mLoading;                   /*!< The block loading, or null. */
    int mSectorsPerBlock;              /*!< Sectors of the disc in one block. */
};

/**
 * The disc cache.
 *
 * @ghidraAddress NTSC-U/C: 0x00492640
 */
extern BlockMgr TheBlockMgr;
