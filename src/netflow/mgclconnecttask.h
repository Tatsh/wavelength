#pragma once

#include "netflow/nettask.h"

/**
 * Step that connects the host to a game server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4c98`, and
 * the object is 0x10 bytes. The step fails with LaunchpadAbortedMsg::kReasonServer.
 */
class MGCLConnectTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00399da8
     * @ghidraAddress PAL: 0x00408630
     */
    ~MGCLConnectTask() override = default;

    /** Values of mServer. */
    enum Server {
        kServerGame = 0,  /*!< The game server HostpadRT::mServerInfo describes. */
        kServerLobby = 1, /*!< The server of the lobby configuration. */
    };

    /**
     * Construct a step for a server.
     *
     * Inline.
     *
     * @param nServer One of Server.
     */
    explicit MGCLConnectTask(int nServer) : mServer(nServer) {
    }

    /**
     * Receive the result of the connection and finish sTask.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nStatus 0 for success, otherwise an error.
     * @ghidraAddress NTSC-U/C: 0x0025b3d8
     * @ghidraAddress PAL: 0x00264428
     */
    static void OnConnect(int nConnection, int nStatus);

    int mServer; /*!< One of Server. */

    /**
     * The step that waits for OnConnect(), or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ad8
     * @ghidraAddress PAL: 0x0042f9a4
     */
    static MGCLConnectTask *sTask;

    /**
     * Non-zero while a connection is under way.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0adc
     * @ghidraAddress PAL: 0x0042f9a8
     */
    static int sConnecting;

protected:
    /**
     * Start connecting to the server.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b1e0
     * @ghidraAddress PAL: 0x00264230
     */
    void OnStart() override;

    /**
     * Clear sTask.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b3c8
     * @ghidraAddress PAL: 0x00264418
     */
    void OnStop() override;
};
