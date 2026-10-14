#pragma once

#include "netflow/nettask.h"

/**
 * Step that reports to the lobby server that the local player is in the lobby.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d53f8`. The
 * step always succeeds.
 */
class UpdateClientStateTask : public NetTask {
protected:
    /**
     * Send the report and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x002644e0
     * @ghidraAddress PAL: 0x0026dd28
     */
    void OnStart() override;
};
