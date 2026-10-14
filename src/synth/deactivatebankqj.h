#pragma once

#include "synth/queuedjob.h"

/**
 * Job that empties a bank slot.
 *
 * The RTTI includes the class name and records QueuedJob as the base.
 */
class DeactivateBankQJ : public QueuedJob {
public:
    /**
     * Construct a job for a slot.
     *
     * @param nSlot The bank slot.
     */
    explicit DeactivateBankQJ(unsigned short nSlot) : QueuedJob(kTypeDeactivateBank), mSlot(nSlot) {
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00396b50
     * @ghidraAddress PAL: 0x00405238
     */
    ~DeactivateBankQJ() override {
    }

    unsigned short mSlot; /*!< The bank slot. */
};
