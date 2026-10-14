#pragma once

#include "os/prnstream.h"

/**
 * Record of one tracked allocation: its sizes, its tags, and the call stack that made it.
 *
 * The class is not polymorphic. The name is inferred. The records come from a BlockPool of
 * 116-byte blocks, and a HeapTracker indexes them by the address of the allocation.
 */
class AllocInfo {
public:
    /** The return addresses a record stores. The last one is always cleared. */
    static constexpr int kStackSize = 24;

    /**
     * Record the return addresses of the caller's caller and the callers above it.
     *
     * @ghidraAddress NTSC-U/C: 0x0029beb8
     * @ghidraAddress PAL: 0x002a5ad8
     */
    void CaptureStack();

    /**
     * Write the tag, the size, and the call stack of the record.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0029be00
     * @ghidraAddress PAL: 0x002a5a20
     */
    void Print(PrnStream &stream) const;

    /**
     * Order records by size, pool size, call stack, tag, and pool tag.
     *
     * The tags compare as greater only when strcmp() reports exactly 1.
     *
     * @param pFirst The first record.
     * @param pSecond The second record.
     * @return Whether pFirst orders before pSecond.
     * @ghidraAddress NTSC-U/C: 0x0029bf28
     * @ghidraAddress PAL: 0x002a5b48
     */
    static bool Less(const AllocInfo *pFirst, const AllocInfo *pSecond);

    int mSizeRequested;              /*!< The bytes the caller requested, 0 for a pool block. */
    int mSizeActual;                 /*!< The bytes the heap handed out. */
    const char *mTag;                /*!< The allocation tag, or null for a pool block. */
    unsigned int mStack[kStackSize]; /*!< Return addresses, ending at the first zero. */
    const char *mPoolTag;            /*!< The tag of the pool block at the address, or null. */
    int mPoolSize;                   /*!< The size of the pool block at the address. */
};
