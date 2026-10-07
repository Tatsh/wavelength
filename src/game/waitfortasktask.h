#pragma once

#include "os/task.h"
#include "os/taskdonenotifier.h"

/**
 * Task that waits until a TaskDoneNotifier reports that named work is done.
 *
 * The RTTI includes the class name and records Task as the base.
 */
class WaitForTaskTask : public Task {
public:
    /**
     * Construct a task that waits for the named work.
     *
     * The task records the name pointer and does not copy the text.
     *
     * @param pszName The name of the work.
     * @param pNotifier The notifier that reports the work.
     * @ghidraAddress NTSC-U/C: 0x001418a0
     * @ghidraAddress PAL: 0x00143240
     */
    WaitForTaskTask(const char *pszName, TaskDoneNotifier *pNotifier);

    TaskDoneNotifier *mNotifier; /*!< The notifier that reports the work. */
    const char *mName;           /*!< The name of the work. */

protected:
    /**
     * Arrange for the notifier to report the work to this task.
     *
     * @ghidraAddress NTSC-U/C: 0x001418c0
     * @ghidraAddress PAL: 0x00143260
     */
    void OnStart() override;
};
