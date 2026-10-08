#pragma once

#include "utl/BlockStat.h"

/**
 * Table of per-name allocation totals.
 *
 * The object is 0x500c bytes. The name comes from the message Update() fails with.
 */
class BlockStatTable {
public:
    /** Capacity of the table. */
    enum { kMaxStats = 1024 };

    /**
     * Create an empty table.
     *
     * @param separateSizes Whether allocations of one name and different sizes get separate totals.
     * @ghidraAddress 0x1001ab70
     */
    explicit BlockStatTable(bool separateSizes);

    /**
     * Empty the table.
     *
     * @ghidraAddress 0x1001aba0
     */
    void Clear();

    /**
     * Report a total.
     *
     * @param iStat The total's index.
     * @return The total.
     * @ghidraAddress 0x1001abb0
     */
    BlockStat &GetBlockStat(int iStat);

    /**
     * Add an allocation to the total of its name, starting a total when none matches.
     *
     * @param name The allocation's name.
     * @param sizeReq The bytes requested.
     * @param sizeActual The bytes allocated.
     * @ghidraAddress 0x1001abf0
     */
    void Update(const char *name, int sizeReq, int sizeActual);

    /**
     * Sort the totals by bytes allocated, largest first.
     *
     * @ghidraAddress 0x1001ad50
     */
    void SortBySize();

    /**
     * Sort the totals by name.
     *
     * @ghidraAddress 0x1001b0c0
     */
    void SortByName();

    /**
     * Report the number of totals.
     *
     * @return The number of totals.
     */
    int NumStats() const {
        return mNumStats;
    }

private:
    BlockStat mStats[kMaxStats]; /*!< The totals. */
    int mMaxStats;               /*!< The capacity, always kMaxStats. */
    int mNumStats;               /*!< The totals in use. */
    bool mSeparateSizes;         /*!< Whether sizes of one name get separate totals. */
};
