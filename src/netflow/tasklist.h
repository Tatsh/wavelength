#pragma once

#include <list>

#include "os/task.h"

/**
 * Tasks that run side by side until each one ends.
 *
 * The class has no RTTI, and the name is inferred. The list owns the tasks.
 */
class TaskList {
public:
    /**
     * Stop and delete every task.
     *
     * @ghidraAddress NTSC-U/C: 0x0026a7c8
     * @ghidraAddress PAL: 0x00274358
     */
    ~TaskList();

    /**
     * Append a task and start it.
     *
     * @param pTask The task.
     * @ghidraAddress NTSC-U/C: 0x0026a730
     * @ghidraAddress PAL: 0x002742c0
     */
    void Add(Task *pTask);

    /**
     * Poll every task, and stop and delete each one that is no longer running.
     *
     * @ghidraAddress NTSC-U/C: 0x0026a888
     * @ghidraAddress PAL: 0x00274418
     */
    void Poll();

    std::list<Task *> mTasks; /*!< The tasks, in the order they were added. */
};
