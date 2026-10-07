#pragma once

#include "os/task.h"
#include "script/dataarray.h"

/**
 * Task that runs one script command.
 *
 * The RTTI includes the class name and records Task as the base.
 */
class ScriptTask : public Task {
public:
    /**
     * Construct a task that runs a command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x00141918
     * @ghidraAddress PAL: 0x001432b8
     */
    explicit ScriptTask(DataArray *pCommand);
};
