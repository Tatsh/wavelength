#pragma once

#include "os/task.h"

/**
 * Task that is done once a set time has passed.
 *
 * The RTTI includes the class name and records Task as the base.
 */
class WaitTimeTask : public Task {
public:
    /**
     * Construct a task that waits for a time.
     *
     * @param fSeconds The time to wait, in seconds.
     * @ghidraAddress NTSC-U/C: 0x00141970
     * @ghidraAddress PAL: 0x00143310
     */
    explicit WaitTimeTask(float fSeconds);
};
