#pragma once

#include "netflow/nettask.h"

/**
 * Step that publishes the settings of the game once HostpadRT::EndEdit() filtered them.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4908`.
 */
class FinishUpdateGameParamsTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a670
     * @ghidraAddress PAL: 0x00408ef8
     */
    ~FinishUpdateGameParamsTask() override = default;

protected:
    /**
     * Send the settings when the host still exists, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025cc90
     * @ghidraAddress PAL: 0x00265cf0
     */
    void OnStart() override;
};
