#pragma once

#include "game/gamecallback.h"
#include "game/tutorialgamelogic.h"
#include "os/task.h"
#include "script/dataarray.h"

/**
 * Task that waits until the player performs the game actions a tutorial step lists.
 *
 * The RTTI includes the class name and records Task and GameCallback as the bases. Only the
 * constructor TutorialGameLogic uses is declared.
 */
class WaitInteractiveTask : public Task, public GameCallback {
public:
    /**
     * Construct a task for one tutorial step.
     *
     * @param pStep The step, with the actions to wait for.
     * @param pLogic The tutorial the step belongs to.
     * @ghidraAddress NTSC-U/C: 0x001419f0
     * @ghidraAddress PAL: 0x00143390
     */
    WaitInteractiveTask(DataArray *pStep, TutorialGameLogic *pLogic);
};
