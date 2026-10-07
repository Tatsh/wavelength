#pragma once

#include "os/task.h"

/**
 * Object a task can ask to report when a named piece of work is done.
 *
 * The RTTI includes the class name, and the class has no base. The vptr is the only member.
 */
class TaskDoneNotifier {
public:
    /**
     * Release the notifier.
     *
     * @ghidraAddress NTSC-U/C: 0x0033fc50
     * @ghidraAddress PAL: 0x003ad188
     */
    virtual ~TaskDoneNotifier() {
    }

    /**
     * Arrange for a task to learn when the named work is done.
     *
     * @param pTask The task waiting for the work.
     * @param pszName The name of the work.
     */
    virtual void NotifyWhenDone(Task *pTask, const char *pszName) = 0;
};
