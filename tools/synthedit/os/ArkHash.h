#pragma once

#include "utl/BinStream.h"

/**
 * Open-addressed hash table of the archive's file and directory names.
 *
 * The object is 0x14 bytes. The class name comes from the allocation name `ArkHash`.
 */
class ArkHash {
public:
    /**
     * Create an empty table.
     *
     * @ghidraAddress 0x1000d380
     */
    ArkHash();

    /**
     * Free the strings and the table.
     *
     * @ghidraAddress 0x1000d3a0
     */
    ~ArkHash();

    /**
     * Find a string.
     *
     * @param str The string.
     * @return The string's slot, or -1 when the table does not have it.
     * @ghidraAddress 0x1000d3c0
     */
    int GetHashValue(const char *str);

    /**
     * Replace the strings and the table with the ones a stream stores.
     *
     * The stream stores the size of the strings, the strings, the number of slots, and one
     * offset into the strings per slot, where zero marks an empty slot.
     *
     * @param stream The stream.
     * @ghidraAddress 0x1000d470
     */
    void Read(BinStream &stream);

    char *mHeap;    /*!< The strings. */
    char *mHeapEnd; /*!< One past the strings. */
    char *mFree;    /*!< One past the strings in use, the same as #mHeapEnd. */
    char **mTable;  /*!< One string per slot, or null for an empty slot. */
    int mTableSize; /*!< The number of slots. */
};
