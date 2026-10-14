#pragma once

#include "synth/queuedjob.h"
#include "synth/synth.h"

/**
 * Job that calls a routine once the jobs before it have run.
 *
 * The RTTI includes the class name and records QueuedJob as the base.
 */
class CallbackQJ : public QueuedJob {
public:
    /**
     * Construct a job.
     *
     * @param pfnCallback The routine.
     * @param nArg The argument passed to the routine.
     */
    CallbackQJ(Synth::Callback pfnCallback, int nArg)
        : QueuedJob(kTypeCallback), mCallback(pfnCallback), mArg(nArg) {
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x00396cc8
     * @ghidraAddress PAL: 0x004053b0
     */
    ~CallbackQJ() override {
    }

    Synth::Callback mCallback; /*!< The routine. */
    int mArg;                  /*!< The argument passed to the routine. */
};
