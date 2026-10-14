#pragma once

#include <vector>

#include "synth/queuedjob.h"

/**
 * Job that divides the sound memory among the bank slots.
 *
 * The RTTI includes the class name and records QueuedJob as the base.
 */
class PartitionQJ : public QueuedJob {
public:
    /**
     * Construct a job with a copy of the sizes.
     *
     * @param blocks The size of each bank slot in 64-byte blocks.
     */
    explicit PartitionQJ(const std::vector<int> &blocks) : QueuedJob(kTypePartition) {
        mBlocks = blocks;
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00396a68
     * @ghidraAddress PAL: 0x00405150
     */
    ~PartitionQJ() override {
    }

    std::vector<int> mBlocks; /*!< The size of each bank slot in 64-byte blocks. */
};
