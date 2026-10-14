#pragma once

#include "netflow/nettask.h"

/**
 * Step that disconnects the host from the game server once no connection is under way.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4c68`.
 */
class MGCLDisconnectTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00399e18
     * @ghidraAddress PAL: 0x004086a0
     */
    ~MGCLDisconnectTask() override = default;

protected:
    /**
     * Disconnect and finish once MGCLConnectTask::sConnecting is clear.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b420
     * @ghidraAddress PAL: 0x00264470
     */
    void OnPoll() override;
};
