#pragma once

#include <vector>

#include "synth/queuedjob.h"

/**
 * Job that sends the packed control changes that set up the hardware effects.
 *
 * The RTTI includes the class name and records QueuedJob as the base. Once the messages are sent,
 * the job waits until the synthesiser module reports the effect update done.
 */
class SetupCoreFxQJ : public QueuedJob {
public:
    /** The polls the job waits before it checks the report of the synthesiser module. */
    static constexpr int kSettlePolls = 30;

    /**
     * Construct a job with a copy of the messages.
     *
     * @param messages The packed messages.
     */
    explicit SetupCoreFxQJ(const std::vector<unsigned int> &messages)
        : QueuedJob(kTypeSetupCoreFx), mWaitPolls(kSettlePolls) {
        mMessages = messages;
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00396e18
     * @ghidraAddress PAL: 0x00405500
     */
    ~SetupCoreFxQJ() override {
    }

    std::vector<unsigned int> mMessages; /*!< The packed messages. */
    int mWaitPolls;                      /*!< The polls left to wait once the messages are sent. */
};
