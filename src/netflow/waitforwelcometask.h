#pragma once

#include "netflow/nettask.h"

/**
 * Step that waits for the host to welcome this console or to turn it away.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4a88`.
 */
class WaitForWelcomeTask : public NetTask {
protected:
    /**
     * Finish once the session is welcomed, and fail with LaunchpadRT::mError once it failed.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c370
     * @ghidraAddress PAL: 0x002653d0
     */
    void OnPoll() override;
};
