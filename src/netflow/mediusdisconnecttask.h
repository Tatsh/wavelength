#pragma once

#include "netflow/nettask.h"

/**
 * Step that disconnects the lobby client from the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5758`.
 */
class MediusDisconnectTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039b988
     * @ghidraAddress PAL: 0x0040a5d0
     */
    ~MediusDisconnectTask() override = default;

protected:
    /**
     * Once MediusConnectTask::sConnecting is clear, disconnect, clear LobbyRT::mMediusConnected,
     * forget the partly received lines of chat, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x002632a8
     * @ghidraAddress PAL: 0x0026c500
     */
    void OnPoll() override;
};
