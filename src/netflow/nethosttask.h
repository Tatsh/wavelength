#pragma once

#include "netflow/nettask.h"

/**
 * Step that starts hosting the connection of the session on the session library.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4b78`.
 * LaunchpadRT::OnNetConnect() finishes the step, which fails with
 * LaunchpadAbortedMsg::kReasonHostFailed.
 */
class NetHostTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a048
     * @ghidraAddress PAL: 0x004088d0
     */
    ~NetHostTask() override = default;

protected:
    /**
     * Start hosting, with LaunchpadRT::sNetTask set to the step.
     *
     * @ghidraAddress NTSC-U/C: 0x0025ba00
     * @ghidraAddress PAL: 0x00264a58
     */
    void OnStart() override;

    /**
     * Clear LaunchpadRT::sNetTask.
     *
     * @ghidraAddress NTSC-U/C: 0x0025bab0
     * @ghidraAddress PAL: 0x00264b08
     */
    void OnStop() override;
};
