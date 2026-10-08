#pragma once

/**
 * A free list of fixed-size elements in one block.
 *
 * The object is 0x10 bytes. The member names follow the assertion text of the source. Which of
 * the two counts is the element size and which the element count is not settled by the control,
 * which never sets up a pool.
 */
class Pool {
public:
    Pool() : mElems(0), mFree(0), mSize(0), mNum(0) {
    }

    /**
     * Take an element.
     *
     * @return The element, or null when none is free.
     * @ghidraAddress 0x1001bad0
     */
    void *Alloc();

    /**
     * Return an element, which must lie in the pool's block.
     *
     * @param elem The element.
     * @ghidraAddress 0x1001baf0
     */
    void Free(void *elem);

private:
    /** An element on the free list; its first word links to the next. */
    struct FreeElem {
        FreeElem *mNext; /*!< The next free element, or null. */
    };

    char *mElems;    /*!< The block of elements. */
    FreeElem *mFree; /*!< The first free element, or null. */
    int mSize;       /*!< The bytes of an element. */
    int mNum;        /*!< The number of elements. */
};
