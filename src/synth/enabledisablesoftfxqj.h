#pragma once

#include "synth/queuedjob.h"

/**
 * Job that turns the software effects on or off.
 *
 * The RTTI includes the class name and records QueuedJob as the base. After sending the request,
 * the job waits a number of polls before it finishes.
 */
class EnableDisableSoftFxQJ : public QueuedJob {
public:
    /** The polls the job waits after sending the request. */
    static constexpr int kSettlePolls = 30;

    /**
     * Construct a job.
     *
     * @param nEnable 1 to turn the software effects on, 0 to turn them off.
     */
    explicit EnableDisableSoftFxQJ(int nEnable)
        : QueuedJob(kTypeEnableDisableSoftFx), mEnable(nEnable), mWaitPolls(kSettlePolls) {
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00396d70
     * @ghidraAddress PAL: 0x00405458
     */
    ~EnableDisableSoftFxQJ() override {
    }

    /** @return 1 to turn the software effects on, 0 to turn them off. */
    int GetEnable() const {
        return mEnable;
    }

    /** @return The polls still to wait once the request is sent. */
    int GetWaitPolls() const {
        return mWaitPolls;
    }

    /**
     * Set the polls still to wait once the request is sent.
     *
     * @param nWaitPolls The polls still to wait.
     */
    void SetWaitPolls(int nWaitPolls) {
        mWaitPolls = nWaitPolls;
    }

private:
    int mEnable;    /*!< 1 to turn the software effects on, 0 to turn them off. */
    int mWaitPolls; /*!< The polls still to wait once the request is sent. */
};
