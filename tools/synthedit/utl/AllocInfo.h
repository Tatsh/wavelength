#pragma once

#include "utl/PrnStream.h"

/**
 * What the memory tracker knows about one allocation.
 *
 * The object is 0x14 bytes and comes from a pool. A heap allocation that a pool carves into nodes
 * records the pool's name as well. The name is inferred from the memory tracker's assertions.
 */
class AllocInfo {
public:
    /**
     * Print the pool name and node size, or else the name and requested size.
     *
     * @param stream The destination.
     * @ghidraAddress 0x10018360
     */
    void Print(PrnStream &stream) const;

    /**
     * Order by requested size, pool node size, name, and pool name.
     *
     * @param other The other record.
     * @return Whether this record sorts first.
     * @ghidraAddress 0x100183d0
     */
    bool operator<(const AllocInfo &other) const;

    int mSizeReq;          /*!< Bytes requested from the heap. */
    int mSizeActual;       /*!< Bytes the heap allocated. */
    const char *mName;     /*!< Name of the heap allocation, or null for a pool node alone. */
    const char *mPoolName; /*!< Name of the pool node, or null. */
    int mPoolSize;         /*!< Size of the pool node in bytes. */
};
