#pragma once

#include "netflow/nettask.h"

/**
 * Step that reports the game of the host to the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4a28`.
 */
class MediusSendWorldReportTask : public NetTask {
protected:
    /**
     * Send the report, whose name encodes the settings of the game, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c3c8
     * @ghidraAddress PAL: 0x00265428
     */
    void OnStart() override;
};
