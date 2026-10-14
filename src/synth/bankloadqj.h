#pragma once

#include "os/string.h"
#include "os/timer.h"
#include "synth/queuedjob.h"

/**
 * Job that loads a bank file into a slot.
 *
 * The RTTI includes the class name and records QueuedJob as the base. The timer measures the
 * load.
 */
class BankLoadQJ : public QueuedJob {
public:
    /**
     * Construct a job for a slot.
     *
     * @param nSlot The bank slot.
     * @param file The bank file.
     */
    BankLoadQJ(unsigned short nSlot, const String &file) : QueuedJob(kTypeBankLoad) {
        mSlot = nSlot;
        mFile = file;
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00396bf8
     * @ghidraAddress PAL: 0x004052e0
     */
    ~BankLoadQJ() override {
    }

    Timer mTimer;         /*!< Measures the load. */
    unsigned short mSlot; /*!< The bank slot. */
    String mFile;         /*!< The bank file. */
};
