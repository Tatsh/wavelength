#pragma once

#include "netflow/nettask.h"

/**
 * Step that waits for the host to finish sharing the remix of the game.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4938`.
 */
class WaitForShareDoneTask : public NetTask {
protected:
    /**
     * Finish once HostpadRT::mSharing is clear.
     *
     * @ghidraAddress NTSC-U/C: 0x0025cc58
     * @ghidraAddress PAL: 0x00265cb8
     */
    void OnPoll() override;
};
