#pragma once

/**
 * Totals of the allocations that share a name.
 *
 * The object is 0x14 bytes. The name is inferred from BlockStatTable.
 */
class BlockStat {
public:
    /**
     * Order by bytes allocated, largest first.
     *
     * @param a The first total.
     * @param b The second total.
     * @return Whether `a` allocated at least as many bytes as `b`.
     * @ghidraAddress 0x1001aaf0
     */
    static bool SizeGreater(const BlockStat &a, const BlockStat &b);

    /**
     * Order by name.
     *
     * @param a The first total.
     * @param b The second total.
     * @return Whether `a`'s name sorts before `b`'s.
     * @ghidraAddress 0x1001ab10
     */
    static bool NameLess(const BlockStat &a, const BlockStat &b);

    const char *mName; /*!< The allocations' name. */
    int mSizeReq;      /*!< Bytes requested, or the one request size when sizes are kept apart. */
    int mSizeActual;   /*!< Bytes allocated, padding included. */
    int mMaxSize;      /*!< The largest request. */
    int mNumAllocs;    /*!< Number of allocations. */
};
