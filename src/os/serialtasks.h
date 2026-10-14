#pragma once

#include <vector>

#include "os/task.h"

/**
 * Task that runs a list of tasks one after another.
 *
 * The RTTI includes the class name and records Task as the base. The constructor has no
 * out-of-line copy. It clears the task list and does not set mCurrent.
 */
class SerialTasks : public Task {
public:
    /**
     * Release the list and every task in it.
     *
     * @ghidraAddress NTSC-U/C: 0x002a0280
     * @ghidraAddress PAL: 0x002a9f38
     */
    ~SerialTasks() override;

    /**
     * Append a task, which the list then manages.
     *
     * @param pTask The task.
     * @ghidraAddress NTSC-U/C: 0x002a0340
     * @ghidraAddress PAL: 0x002a9ff8
     */
    void Add(Task *pTask);

protected:
    /**
     * Start the first task, or finish with success when the list is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x002a04d8
     * @ghidraAddress PAL: 0x002aa190
     */
    void OnStart() override;

    /**
     * Stop the task that runs now.
     *
     * @ghidraAddress NTSC-U/C: 0x002a0520
     * @ghidraAddress PAL: 0x002aa1d8
     */
    void OnStop() override;

    /**
     * Poll the task that runs now, start the next one after it is done, and finish after the last
     * one or after a failure.
     *
     * @ghidraAddress NTSC-U/C: 0x002a0560
     * @ghidraAddress PAL: 0x002aa218
     */
    void OnPoll() override;

public:
    std::vector<Task *> mTasks;             /*!< The tasks, in the order they run. */
    std::vector<Task *>::iterator mCurrent; /*!< The task that runs now. */
};
