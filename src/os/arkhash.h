#pragma once

#include "os/binstream.h"

/**
 * Hash table of the names of the files and directories of an archive.
 *
 * The class is not polymorphic and emits no RTTI. The name is taken from the tag of its
 * allocations. The table stores the offset of each name in the string block, and zero marks an
 * empty slot.
 */
class ArkHash {
public:
    /**
     * Construct an empty table.
     *
     * @ghidraAddress NTSC-U/C: 0x0028df60
     * @ghidraAddress PAL: 0x00297940
     */
    ArkHash();

    /**
     * Find the slot of a name, probing the following slots in turn.
     *
     * @param pszName The name.
     * @return The slot, or -1 when the name is absent.
     * @ghidraAddress NTSC-U/C: 0x0028df80
     * @ghidraAddress PAL: 0x00297960
     */
    int GetHashValue(const char *pszName) const;

    /**
     * Replace the table with the one a stream stores.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0028e038
     * @ghidraAddress PAL: 0x00297a18
     */
    void Read(BinStream &stream);

    char *mStrings;      /*!< The string block. */
    char *mStringsEnd;   /*!< The end of the string block. */
    char *mStringsLimit; /*!< The end of the allocation of the string block. */
    int *mTable;         /*!< The offset of each slot's name in mStrings, or 0. */
    int mTableSize;      /*!< The number of slots. */
};
