#pragma once

#include "netflow/nettask.h"

/**
 * Step that stops the game server library.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4cf8`.
 */
class CloseMGCLTask : public NetTask {
protected:
    /**
     * Stop the library, note that it is no longer ready, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b158
     * @ghidraAddress PAL: 0x002641a8
     */
    void OnStart() override;
};
