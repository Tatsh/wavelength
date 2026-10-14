#pragma once

/**
 * Request SynthPS2 queues and runs in order once the sound processor is free.
 *
 * The RTTI includes the class name and records no base. The two data members precede the vptr.
 * SynthPS2::ProcessJobs() picks the job's class from mType and runs the job at the front of the
 * queue, polling it on later frames until it finishes.
 */
class QueuedJob {
public:
    /** The kinds of job, the values of mType. */
    enum Type {
        kTypePartition = 0,           /*!< A PartitionQJ. */
        kTypeDeactivateBank = 1,      /*!< A DeactivateBankQJ. */
        kTypeBankLoad = 2,            /*!< A BankLoadQJ. */
        kTypeEnableDisableSoftFx = 3, /*!< An EnableDisableSoftFxQJ. */
        kTypeSetupCoreFx = 4,         /*!< A SetupCoreFxQJ. */
        kTypeCallback = 5,            /*!< A CallbackQJ. */
    };

    /**
     * Construct a job that has not started.
     *
     * @param nType The kind of job, one of Type.
     */
    explicit QueuedJob(int nType) : mStarted(0), mType(nType) {
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00397008
     * @ghidraAddress PAL: 0x004056f0
     */
    virtual ~QueuedJob() {
    }

    int mStarted; /*!< 1 once the job has sent its request. */
    int mType;    /*!< One of Type. */
};
