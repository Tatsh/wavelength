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

    std::vector<Task *> mTasks;             /*!< The tasks, in the order they run. */
    std::vector<Task *>::iterator mCurrent; /*!< The task that runs now. */
};
