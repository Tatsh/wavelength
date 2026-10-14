#pragma once

#include <librtmgcl.h>

#include "netflow/nettask.h"

/**
 * Step that authenticates the host with the game server, which gives the server to host on.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4bd8`. The
 * step fails with LaunchpadAbortedMsg::kReasonServer.
 */
class MGCLAuthenticationTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x00399f68
     * @ghidraAddress PAL: 0x004087f0
     */
    ~MGCLAuthenticationTask() override = default;

    /**
     * Take the game server into HostpadRT::mServerInfo and finish the step.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x0025b860
     * @ghidraAddress PAL: 0x002648b8
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMgclAuthenticationResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b7f8
     * @ghidraAddress PAL: 0x00264850
     */
    void OnStart() override;
};
