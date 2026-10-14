#pragma once

#include "netflow/nettask.h"

/**
 * Step that waits for the key of the session from the host.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4a58`.
 */
class WaitForKeyTask : public NetTask {
protected:
    /**
     * Finish once JoinpadRT::mKeyReceived is set.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c338
     * @ghidraAddress PAL: 0x00265398
     */
    void OnPoll() override;
};
