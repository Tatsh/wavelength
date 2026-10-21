#pragma once

#include <stddef.h>

#include "os/prnstream.h"

/** Totals of the tracked allocations of one tag. */
struct BlockStat {
    const char *mName;  /*!< The tag. */
    int mSizeRequested; /*!< The bytes requested. */
    int mSizeActual;    /*!< The bytes handed out. */
    int mMaxRequested;  /*!< The largest request. */
    int mNumAllocs;     /*!< The number of allocations. */
};

/**
 * Table of allocation totals by tag, filled from a HeapTracker for a report.
 *
 * The class is not polymorphic. The name is the one the overflow warning gives.
 */
class BlockStatTable {
public:
    /** The rows a table has room for. */
    static constexpr int kCapacity = 1024;

    /**
     * Construct an empty table.
     *
     * @param bSeparateSizes Non-zero to give each request size of a tag a separate row.
     * @ghidraAddress NTSC-U/C: 0x0029c7b0
     * @ghidraAddress PAL: 0x002a63d8
     */
    explicit BlockStatTable(int bSeparateSizes);

    /**
     * Take a table from the memory tracker's own region.
     *
     * @param nSize The size of the table.
     * @return The table.
     */
    static void *operator new(size_t nSize);

    /**
     * Ignore the release of a table. The memory tracker's region never takes memory back.
     */
    static void operator delete(void *) noexcept {
    }

    /**
     * Empty the table.
     *
     * @ghidraAddress NTSC-U/C: 0x0029c7c8
     * @ghidraAddress PAL: 0x002a63f0
     */
    void Clear();

    /**
     * Report a row.
     *
     * @param nIndex The row.
     * @return The row.
     * @ghidraAddress NTSC-U/C: 0x0029c7d0
     * @ghidraAddress PAL: 0x002a63f8
     */
    BlockStat *GetStat(int nIndex);

    /**
     * Add an allocation to the row of its tag, adding the row when the tag is new.
     *
     * A full table reports an overflow through DebugWarn().
     *
     * @param pszName The tag.
     * @param nRequested The bytes requested.
     * @param nActual The bytes handed out.
     * @ghidraAddress NTSC-U/C: 0x0029c7e0
     * @ghidraAddress PAL: 0x002a6408
     */
    void Update(const char *pszName, int nRequested, int nActual);

    /**
     * Sort the rows by bytes handed out, largest first.
     *
     * @ghidraAddress NTSC-U/C: 0x0029c980
     * @ghidraAddress PAL: 0x002a65a8
     */
    void SortBySize();

    /**
     * Sort the rows by tag.
     *
     * @ghidraAddress NTSC-U/C: 0x0029c9b0
     * @ghidraAddress PAL: 0x002a65d8
     */
    void SortByName();

    /**
     * Write the differences between the rows of two tables sorted by tag.
     *
     * Rows past the end of the shorter table are left out.
     *
     * @param stream The stream to write to.
     * @param pCurrent The newer table.
     * @param pPrevious The older table.
     * @ghidraAddress NTSC-U/C: 0x0029c110
     * @ghidraAddress PAL: 0x002a5d30
     */
    static void PrintDelta(PrnStream &stream, BlockStatTable *pCurrent, BlockStatTable *pPrevious);

    /** @return The rows in use. */
    int GetCount() const {
        return mCount;
    }

private:
    // Order rows by bytes handed out, largest first, with equal rows in either order.
    // NTSC-U/C: 0x0029c770, PAL: 0x002a6398
    static bool SizeNotLess(const BlockStat &first, const BlockStat &second);

    // Order rows by tag.
    // NTSC-U/C: 0x0029c788, PAL: 0x002a63b0
    static bool NameLess(const BlockStat &first, const BlockStat &second);

    BlockStat mStats[kCapacity]; /*!< The rows. */
    int mCapacity;               /*!< kCapacity. */
    int mCount;                  /*!< The rows in use. */
    int mSeparateSizes; /*!< Non-zero when each request size of a tag has a separate row. */
};
