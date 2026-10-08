#pragma once

/**
 * Header of a free run of words in a heap.
 *
 * The header occupies the first two words of the run. A heap links its free runs in a singly linked
 * list sorted by address. The name is inferred from the heap's assertions.
 */
class FreeBlock {
public:
    /**
     * Report the word after the run.
     *
     * @return The first word past the run.
     */
    int *EndAddr() {
        return reinterpret_cast<int *>(this) + mSizeWords;
    }

    /**
     * Absorb the next free run when it starts where this run ends.
     *
     * @param next The run after this one in the free list.
     * @return Whether the runs were merged.
     * @ghidraAddress 0x10010560
     */
    bool Coalesce(FreeBlock *next);

    int mSizeWords;   /*!< Length of the run in words, including this header. */
    FreeBlock *mNext; /*!< The next free run at a higher address, or null. */
};
