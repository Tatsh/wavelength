#pragma once

#include "netflow/nettask.h"

/**
 * Step that ends the online session of this console as lost.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4cc8`.
 */
class KillLaunchpadTask : public NetTask {
protected:
    /**
     * Abort the session with LaunchpadAbortedMsg::kReasonLost when there is one, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b198
     * @ghidraAddress PAL: 0x002641e8
     */
    void OnStart() override;
};
