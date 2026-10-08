#pragma once

/**
 * Fixed pool of effects, handed out from a free list and stolen oldest first when it runs dry.
 *
 * The RTTI names the template. The object is 0x10 bytes, with the vptr after the members. An
 * effect type supplies mNextFree, mStartTime, Start(), and Stop().
 *
 * @tparam T The effect.
 * @tparam bArg The second template argument. Its meaning is not yet recovered.
 */
template <class T, bool bArg>
class FxPool {
public:
    /**
     * Construct a pool of effects, every one of them free.
     *
     * @param nCount The number of effects.
     * @ghidraAddress NTSC-U/C: 0x00367da8
     * @ghidraAddress PAL: 0x003d64d8
     */
    explicit FxPool(int nCount) : mCount(nCount) {
        mItems = new T[nCount];
        mFree = mItems;
        for (int i = 0; i < nCount; ++i) {
            mFree[i].mNextFree = i == nCount - 1 ? nullptr : &mFree[i + 1];
        }
    }

    /**
     * Destroy the effects.
     *
     * @ghidraAddress NTSC-U/C: 0x00367eb0
     * @ghidraAddress PAL: 0x003d65e0
     */
    virtual ~FxPool() {
        delete[] mItems;
    }

    /**
     * Report the effect that started first.
     *
     * @return The effect, or null for an empty pool.
     * @ghidraAddress NTSC-U/C: 0x00368050
     * @ghidraAddress PAL: 0x003d6780
     */
    T *FindOldest() {
        if (mCount == 0) {
            return nullptr;
        }
        T *pOldest = mItems;
        for (T *pItem = mItems + 1; pItem != mItems + mCount; ++pItem) {
            if (pItem->mStartTime < pOldest->mStartTime) {
                pOldest = pItem;
            }
        }
        return pOldest;
    }

    /**
     * Take an effect off the free list.
     *
     * @return The effect, or null when every effect is in use.
     * @ghidraAddress NTSC-U/C: 0x003680b0
     * @ghidraAddress PAL: 0x003d67e0
     */
    T *Pop() {
        T *pItem = mFree;
        if (pItem == nullptr) {
            return nullptr;
        }
        mFree = pItem->mNextFree;
        pItem->mNextFree = nullptr;
        return pItem;
    }

    /**
     * Take an effect off the free list and start it.
     *
     * @param args The arguments of the effect's Start().
     * @return The effect, or null when every effect is in use.
     * @ghidraAddress NTSC-U/C: 0x003680d8
     * @ghidraAddress PAL: 0x003d6808
     */
    template <class... Args>
    T *Start(Args... args) {
        T *pItem = Pop();
        if (pItem != nullptr) {
            pItem->Start(args...);
        }
        return pItem;
    }

    /**
     * Stop an effect and return it to the free list.
     *
     * @param pItem The effect.
     * @ghidraAddress NTSC-U/C: 0x00368150
     * @ghidraAddress PAL: 0x003d6880
     */
    void Release(T *pItem) {
        pItem->Stop();
        pItem->mNextFree = mFree;
        mFree = pItem;
    }

    /**
     * Return an effect to the pool.
     *
     * @param pItem The effect.
     * @ghidraAddress NTSC-U/C: 0x00368190
     * @ghidraAddress PAL: 0x003d68c0
     */
    void Free(T *pItem) {
        Release(pItem);
    }

    int mCount; /*!< The number of effects. */
    T *mFree;   /*!< The head of the free list. */
    T *mItems;  /*!< The effects. */
};
