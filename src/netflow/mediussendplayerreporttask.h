#pragma once

#include "netflow/nettask.h"

/**
 * Step that reports the player of this console in the game to the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d49f8`.
 */
class MediusSendPlayerReportTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a3c8
     * @ghidraAddress PAL: 0x00408c50
     */
    ~MediusSendPlayerReportTask() override = default;

protected:
    /**
     * Send the report, whose statistics text encodes the player, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c5d0
     * @ghidraAddress PAL: 0x00265630
     */
    void OnStart() override;
};
