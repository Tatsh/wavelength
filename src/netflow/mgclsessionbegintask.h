#pragma once

#include <librtmgcl.h>

#include "netflow/nettask.h"

/**
 * Step that begins a session of the host with the game server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4c38`. The
 * step fails with LaunchpadAbortedMsg::kReasonServer.
 */
class MGCLSessionBeginTask : public NetTask {
public:
    /**
     * Take the key of the session into HostpadRT::mSessionKey and finish the step.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x0025b4f0
     * @ghidraAddress PAL: 0x00264548
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMgclSessionBeginResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b470
     */
    void OnStart() override;
};
