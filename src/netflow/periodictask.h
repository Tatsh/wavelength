#pragma once

#include "os/task.h"

/**
 * A task that runs again a fixed time after each run ends.
 *
 * The class has no RTTI, and the name is inferred. The object is 0xc bytes.
 */
class PeriodicTask {
public:
    /** The time a schedule that waits for nothing records. */
    static constexpr float kNever = 1.0e30f;

    /**
     * Construct a schedule with no task.
     *
     * @param fPeriod The wait after each run, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0026a588
     * @ghidraAddress PAL: 0x00274118
     */
    explicit PeriodicTask(float fPeriod);

    /**
     * Stop and delete the task.
     *
     * @ghidraAddress NTSC-U/C: 0x0026a5a8
     * @ghidraAddress PAL: 0x00274138
     */
    ~PeriodicTask();

    /**
     * Set the task, which the schedule then deletes.
     *
     * @param pTask The task.
     * @ghidraAddress NTSC-U/C: 0x0026a618
     * @ghidraAddress PAL: 0x002741a8
     */
    void SetTask(Task *pTask);

    /**
     * Start the task once its time comes, poll it, and schedule the next run once it ends.
     *
     * @return The state of the task, or 0 with no task.
     * @ghidraAddress NTSC-U/C: 0x0026a620
     * @ghidraAddress PAL: 0x002741b0
     */
    int Poll();

    /**
     * Run the task at the next Poll().
     *
     * @ghidraAddress NTSC-U/C: 0x0026a710
     * @ghidraAddress PAL: 0x002742a0
     */
    void RunNow();

    /**
     * Stop the schedule from starting the task.
     *
     * @ghidraAddress NTSC-U/C: 0x0026a718
     * @ghidraAddress PAL: 0x002742a8
     */
    void Suspend();

    Task *mTask;     /*!< The task, or null. */
    float mPeriod;   /*!< The wait after each run, in milliseconds. */
    float mNextTime; /*!< When the task starts next, from SystemMs(), or kNever. */
};
