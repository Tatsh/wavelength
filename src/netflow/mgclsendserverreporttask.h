#pragma once

#include "netflow/nettask.h"

/**
 * Step that reports the host to the game server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4ba8`.
 */
class MGCLSendServerReportTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00399fd8
     * @ghidraAddress PAL: 0x00408860
     */
    ~MGCLSendServerReportTask() override = default;

protected:
    /**
     * Send the report and finish.
     *
     * The host reports one game of one player of kMaxClients.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b988
     * @ghidraAddress PAL: 0x002649e0
     */
    void OnStart() override;
};
