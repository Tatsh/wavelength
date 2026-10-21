#pragma once

#include "memcard/mcgetdirtask.h"
#include "memcard/mcgetinfotask.h"
#include "memcard/memcardserialtask.h"

/** The listings MCInitialCheckTask makes: the Freq directory, then its Freq files. */
constexpr int kInitialCheckListingCount = 2;

/**
 * Work that checks a card at startup: read the card information, then list the Freq directory and
 * its Freq files, and measure the space one more Freq needs.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x34 bytes. Another
 * card than before counts as success. The result is MemcardTask::kStatusFull when the card lacks
 * the space.
 */
class MCInitialCheckTask : public MemcardSerialTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00352350
     * @ghidraAddress PAL: 0x003bf5d0
     */
    ~MCInitialCheckTask() override = default;

    /**
     * Construct the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e688
     * @ghidraAddress PAL: 0x0015fec8
     */
    MCInitialCheckTask();

    /**
     * Prepare the work, and make g_MemcardPath the Freq directory.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015e7b8
     * @ghidraAddress PAL: 0x0015fff8
     */
    void Set(int nPort);

    /**
     * Add the space a missing directory or a missing Freq needs, and compare the total with the
     * free space once both listings ended.
     *
     * @ghidraAddress NTSC-U/C: 0x0015e8c8
     * @ghidraAddress PAL: 0x00160108
     */
    void OnDirListed() override;

    /** @return The step that reads the card information. */
    MCGetInfoTask *GetGetInfo() const {
        return mGetInfo;
    }

    /** @return The kilobytes one more Freq needs. */
    int GetNeeded() const {
        return mNeeded;
    }

private:
    MCGetInfoTask *mGetInfo; /*!< The step that reads the card information. Borrowed. */
    // The step above belongs to the SerialTasks list.
    MCGetDirTask *mGetDirs[kInitialCheckListingCount]; /*!< The listings. Borrowed. */
    int mNeeded;                                       /*!< The kilobytes one more Freq needs. */
    unsigned char mListing;                            /*!< The listing that ends next. */
};
