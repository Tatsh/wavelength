#pragma once

#include "netflow/nettask.h"

/**
 * Step that closes the connection of the session on the session library from the worker thread.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4ab8`.
 */
class NetDisconnectTask : public NetTask {
public:
    /**
     * Close LaunchpadRT::mConnection, on the worker thread.
     *
     * The name is inferred.
     *
     * @return The result of the session library.
     * @ghidraAddress NTSC-U/C: 0x0025c2a8
     * @ghidraAddress PAL: 0x00265308
     */
    static int Disconnect();

    /**
     * Clear LaunchpadRT::mConnection, report that the player left the game, and finish sTask.
     *
     * The name is inferred.
     *
     * @param nResult The result of Disconnect(), which is not read.
     * @ghidraAddress NTSC-U/C: 0x0025c2d0
     * @ghidraAddress PAL: 0x00265330
     */
    static void OnDisconnected(int nResult);

    /**
     * The step that waits for OnDisconnected(), or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ae8
     */
    static NetDisconnectTask *sTask;

protected:
    /**
     * Queue Disconnect() for the worker thread.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c278
     * @ghidraAddress PAL: 0x002652d8
     */
    void OnStart() override;
};
