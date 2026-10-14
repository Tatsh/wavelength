#pragma once

#include "netflow/nettask.h"

/**
 * Step that connects the lobby client to a lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5788` and the
 * object is 0x10 bytes. The step sets LobbyRT::mMediusConnected, and it fails with
 * LobbyConnectResultMsg::kResultConnectFailed.
 */
class MediusConnectTask : public NetTask {
public:
    /** Values of mServer. */
    enum Server {
        kServerLocation = 0, /*!< The server LobbyRT::mLobbyInfo identifies. */
        kServerLobby = 1,    /*!< The server `server.cfg` identifies. */
    };

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param nServer One of Server.
     */
    explicit MediusConnectTask(int nServer) : mServer(nServer) {
    }

    /**
     * Take the result of the connection and finish sTask.
     *
     * The name is inferred.
     *
     * @param nConnection The connection, or -1 when the connection failed.
     * @ghidraAddress NTSC-U/C: 0x00263250
     * @ghidraAddress PAL: 0x0026c4a8
     */
    static void OnConnect(int nConnection);

    /**
     * The step that waits for OnConnect(), or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0afc
     */
    static MediusConnectTask *sTask;

    /**
     * Non-zero while a connection is being made.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0b00
     */
    static int sConnecting;

    int mServer; /*!< One of Server. */

protected:
    /**
     * Start connecting.
     *
     * @ghidraAddress NTSC-U/C: 0x00262fa0
     * @ghidraAddress PAL: 0x0026c0f8
     */
    void OnStart() override;

    /**
     * Clear sTask.
     *
     * @ghidraAddress NTSC-U/C: 0x00263240
     */
    void OnStop() override;
};
