#pragma once

#include <librtmgcl.h>

#include "netflow/nettask.h"

/**
 * Step that creates the game of the host on the game server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4b18`. The
 * step fails with LaunchpadAbortedMsg::kReasonInit when the request cannot be sent, and with
 * LaunchpadAbortedMsg::kReasonCreateFailed when the server refuses the game.
 */
class MGCLCreateGameTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a128
     * @ghidraAddress PAL: 0x004089b0
     */
    ~MGCLCreateGameTask() override = default;

    /**
     * Take the game into LaunchpadRT::mWorldId and finish sTask.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x0025c060
     * @ghidraAddress PAL: 0x002650c0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMgclCreateGameResponse *pResponse);

    /**
     * The step that waits for OnResponse(), or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ae4
     * @ghidraAddress PAL: 0x0042f9b0
     */
    static MGCLCreateGameTask *sTask;

protected:
    /**
     * Send the request, whose name encodes the settings of the game.
     *
     * @ghidraAddress NTSC-U/C: 0x0025bec8
     * @ghidraAddress PAL: 0x00264f20
     */
    void OnStart() override;
};
