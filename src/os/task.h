#pragma once

/**
 * Unit of work an owner starts, polls until it reports that it is done, and stops.
 *
 * The RTTI includes the class name, and the class has no base. One data word, the state, precedes
 * the vptr.
 */
class Task {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,    /*!< Not started, or stopped. */
        kStateRunning = 1, /*!< Started and not yet done. */
        kStateDone = 2,    /*!< Done, set by an override of OnPoll() or by Finish(). */
        kStateFailed = 3,  /*!< Ended without success by Finish(). */
    };

    /**
     * Construct an idle task.
     *
     * The constructor has no out-of-line copy.
     */
    Task() : mState(kStateIdle) {
    }

    /**
     * Release the task.
     *
     * @ghidraAddress NTSC-U/C: 0x002a0130
     * @ghidraAddress PAL: 0x002a9de8
     */
    virtual ~Task();

    /**
     * Report the state after giving a running task one poll.
     *
     * @return One of State.
     * @ghidraAddress NTSC-U/C: 0x002a01a8
     * @ghidraAddress PAL: 0x002a9e60
     */
    int Poll();

    /**
     * Enter the running state and call OnStart(), unless the task is already running.
     *
     * @ghidraAddress NTSC-U/C: 0x002a01f8
     * @ghidraAddress PAL: 0x002a9eb0
     */
    void Start();

    /**
     * Call OnStop() when the task is running, and return to the idle state.
     *
     * @ghidraAddress NTSC-U/C: 0x002a0238
     * @ghidraAddress PAL: 0x002a9ef0
     */
    void Stop();

    /**
     * End the task from outside its poll, and call OnStop() when it was running.
     *
     * The name is inferred.
     *
     * @param bSucceeded Enter kStateDone rather than kStateFailed.
     * @ghidraAddress NTSC-U/C: 0x002a0160
     * @ghidraAddress PAL: 0x002a9e18
     */
    void Finish(bool bSucceeded);

    /** One of State. */
    int mState;

protected:
    /** Begin the work. Start() calls this member. */
    virtual void OnStart() {
    }

    /** Abandon the work. Stop() calls this member for a running task. */
    virtual void OnStop() {
    }

    /** Advance the work, and set mState to kStateDone when it finishes. */
    virtual void OnPoll() {
    }
};
