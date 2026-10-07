#pragma once

#include "os/task.h"

/**
 * Task that is done once a set time has passed on the song clock.
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

    float mDuration; /*!< The time to wait, in milliseconds. */
    float mEndTime;  /*!< The song clock time the task is done at, in milliseconds. */

protected:
    /**
     * Set the end time from the song clock.
     *
     * @ghidraAddress NTSC-U/C: 0x001419a0
     */
    void OnStart() override;

    /**
     * Finish the task once the song clock reaches the end time.
     *
     * @ghidraAddress NTSC-U/C: 0x001419b8
     * @ghidraAddress PAL: 0x00143358
     */
    void OnPoll() override;
};
