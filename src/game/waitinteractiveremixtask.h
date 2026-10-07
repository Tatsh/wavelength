#pragma once

#include "game/gamecallback.h"
#include "game/remixtutorial.h"
#include "os/task.h"
#include "script/dataarray.h"

/**
 * Task that waits until the player performs the remix actions a tutorial step lists.
 *
 * The RTTI includes the class name and records Task and GameCallback as the bases.
 */
class WaitInteractiveRemixTask : public Task, public GameCallback {
public:
    /**
     * Construct a task for one tutorial step.
     *
     * @param pStep The step, with the actions to wait for.
     * @param pTutorial The tutorial the step belongs to.
     * @ghidraAddress NTSC-U/C: 0x00142290
     * @ghidraAddress PAL: 0x00143c30
     */
    WaitInteractiveRemixTask(DataArray *pStep, RemixTutorial *pTutorial);
};
